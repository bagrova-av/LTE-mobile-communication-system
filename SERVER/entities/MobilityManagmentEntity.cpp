#include "MobilityManagmentEntity.h"
#include "../../COMMON/constants.h"
#include "../../COMMON/logging/Logger.h"
#include "../../CORE/network/SerializerFactory.h"

MobilityManagmentEntity::MobilityManagmentEntity(const MmeConfig& config,
                                                std::vector<std::unique_ptr<BaseStation>> baseStations,
                                                std::unique_ptr<HlrEirInterface> hlrEir,
                                                std::unique_ptr<SmsCenter> smsCenter) :
    config(config),
    baseStations(std::move(baseStations)),
    homeLocationAndEquipmentIdentityRegister(std::move(hlrEir)),
    smsCenter(std::move(smsCenter))
{ }

BaseStation* MobilityManagmentEntity::getBaseStationByIdWithoutMutex(BaseStationId baseStationId)
{
    for (auto& baseStation : baseStations)
    {
        if (baseStation->getId() == baseStationId)
        {
            return baseStation.get();
        }
    }
    return nullptr;
}

BaseStation* MobilityManagmentEntity::getBaseStationById(BaseStationId baseStationId)
{
    std::lock_guard<std::mutex> lock(mmeMutex);
    return getBaseStationByIdWithoutMutex(baseStationId);
}

BaseStation* MobilityManagmentEntity::getBaseStationByTmsiWithoutMutex(M_TMSI m_tmsi)
{
    auto itClientData = visitorLocationRegister.find(m_tmsi);
    if (itClientData == visitorLocationRegister.end())
    {
        return nullptr;
    }

    return getBaseStationByIdWithoutMutex(itClientData->second.currentBaseSationId);
}

BaseStation* MobilityManagmentEntity::getBaseStationByTmsi(M_TMSI m_tmsi)
{
    std::lock_guard<std::mutex> lock(mmeMutex);
    return getBaseStationByTmsiWithoutMutex(m_tmsi);
}

M_TMSI MobilityManagmentEntity::generateTMSI()
{
    return nextTMSI++;
}

std::optional<BaseStationId> MobilityManagmentEntity::findBestBaseStation(const LocationPoint3D& clientLocation)
{
    std::lock_guard<std::mutex> lock(mmeMutex);

    if (baseStations.empty())
    {
        return std::nullopt;
    }

    float maxSignalStrength = std::numeric_limits<float>::lowest();
    std::optional<BaseStationId> bestBaseStationId;
    for (const auto& baseStation : baseStations)
    {
        float signalStrength = baseStation->calculateSignalStrength(clientLocation);
        if (signalStrength >= MINIMAL_SIGNAL_STRENGTH)
        {
            if (signalStrength > maxSignalStrength)
            {
                maxSignalStrength = signalStrength;
                bestBaseStationId = baseStation->getId();
            }
        }
    }
    return bestBaseStationId;
}

void MobilityManagmentEntity::cleanupExpiredMessages()
{
    smsCenter->processExpiredMessages();

    std::lock_guard<std::mutex> lock(mmeMutex);
    for (auto& [tmsi, data] : visitorLocationRegister)
    {
        const auto& events = smsCenter->popEventsForSubscriber(data.msisdn);
        
        for (const auto& sms : events.statusReports)
        {
            SmsDeliveryStatus status;
            if (sms.status == SmsStatus::WAITING_DELIVERY_REPORT)
            {
                status = SmsDeliveryStatus::DELIVERED;
            }
            else if (sms.status == SmsStatus::WAITING_FAILURE_REPORT)
            {
                status = SmsDeliveryStatus::FAILED;
            }
            
            auto currentSerializer = SerializerFactory::createSerializer(data.protocol);
            BaseStation* baseStation = getBaseStationByIdWithoutMutex(data.currentBaseSationId);
            if (baseStation)
            {
                SmsStatusReport smsStatusReport{sms.senderTmsi, sms.smsId, status};
                NetworkDataPacket packetForBuffer{static_cast<uint8_t>(data.protocol),
                                               PacketType::SMS_STATUS,
                                              currentSerializer->serialize(smsStatusReport)};
                baseStation->pushMessageToBuffer(sms.senderTmsi, packetForBuffer);
                Logger::info(std::format("MME: Expired SMS with ID = {} report sent to sender with M_TMSI = {}",
                                         sms.smsId, sms.senderTmsi));
            }
        }
    }
}

std::optional<M_TMSI> MobilityManagmentEntity::attachClient(const IMEI& imei, const IMSI& imsi,
                                                            BaseStationId baseStationId, 
                                                            int clientFileDescriptor,
                                                            Protocol protocol)
{
    std::lock_guard<std::mutex> lock(mmeMutex);

    auto it = imsiLookupTmsi.find(imsi);
    if (it != imsiLookupTmsi.end())
    {
        bool clientDetached = detachClientWithoutMutex(it->second);
        if (!clientDetached)
        {
            Logger::warning("Error: failed to disconnect the client with M_TMSI = " + std::to_string(it->second));
            return std::nullopt;
        }
    }

    std::optional<HlrEirRecord> subscriberRecord = homeLocationAndEquipmentIdentityRegister->getSubscriberByImsi(imsi);
    if (!subscriberRecord.has_value())
    {
        Logger::info("The client was not found in the HLR(EIR) database");
        return std::nullopt;
    }

    if (subscriberRecord.value().identityData.imei != imei)
    {
        Logger::warning(std::format("Error: the client with an IMSI ({}) and IMEI ({}) "
                                    "in the HLR(EIR) has a different IMEI({})", 
                                    imsi, imei, subscriberRecord.value().identityData.imei));
        return std::nullopt;
    }

    BaseStation* targetBaseStation = getBaseStationByIdWithoutMutex(baseStationId);
    if (!targetBaseStation)
    {
        return std::nullopt;
    }

    if (visitorLocationRegister.size() >= config.maxVlrRecords)
    {
        Logger::warning("MME: Attach rejected because VLR limit reached");
        return std::nullopt;
    }
    M_TMSI m_tmsi = generateTMSI();
    if (!targetBaseStation->registerClient(m_tmsi, clientFileDescriptor))
    {
        Logger::warning("Error: failed to register the client at the base station with ID = "
                        + std::to_string(baseStationId));
        return std::nullopt;
    }

    homeLocationAndEquipmentIdentityRegister->updateSubscriberSessionData(imsi, config.id,
                                                                         std::chrono::system_clock::now());
    homeLocationAndEquipmentIdentityRegister->updateSubscriberStatus(imsi, SubscriberStatus::ACTIVE);
    
    visitorLocationRegister.emplace(m_tmsi, ClientData{imei,
                                                        subscriberRecord.value().identityData.msisdn,
                                                        baseStationId,
                                                        protocol}
    );
    msisdnLookupTmsi.emplace(subscriberRecord.value().identityData.msisdn, m_tmsi);
    imsiLookupTmsi.emplace(imsi, m_tmsi);
    tmsiLookupImsi.emplace(m_tmsi,imsi);

    // Checking messages for this client in SMSC
    smsCenter->processExpiredMessages();
    auto events = smsCenter->popEventsForSubscriber(subscriberRecord->identityData.msisdn);
    auto currentSerializer = SerializerFactory::createSerializer(protocol);
    for (const auto& sms : events.incomingMessages)
    {
        SmsIncomingMessage smsIncomingMessage{m_tmsi, sms.senderMsisdn, sms.smsId, sms.text};
        NetworkDataPacket packetForBuffer{static_cast<uint8_t>(protocol),
                                       PacketType::SMS_RECEIVE,
                                       currentSerializer->serialize(smsIncomingMessage)};
        targetBaseStation->pushMessageToBuffer(m_tmsi, packetForBuffer);
    }

    for (const auto& sms : events.statusReports)
    {
        SmsDeliveryStatus status;
        if (sms.status == SmsStatus::WAITING_DELIVERY_REPORT)
        {
            status = SmsDeliveryStatus::DELIVERED;
        }
        else if (sms.status == SmsStatus::WAITING_FAILURE_REPORT)
        {
            status = SmsDeliveryStatus::FAILED;
        }
        SmsStatusReport report{m_tmsi, sms.smsId, status};
        NetworkDataPacket packetForBuffer{static_cast<uint8_t>(protocol),
                                      PacketType::SMS_STATUS,
                                       currentSerializer->serialize(report)};
        targetBaseStation->pushMessageToBuffer(m_tmsi, packetForBuffer);
    }

    return m_tmsi;
}

bool MobilityManagmentEntity::detachClientWithoutMutex(M_TMSI m_tmsi)
{
    auto iteratorVlr = visitorLocationRegister.find(m_tmsi);
    if (iteratorVlr == visitorLocationRegister.end())
    {
        return false;
    }
    
    BaseStation* baseStation = getBaseStationByIdWithoutMutex(iteratorVlr->second.currentBaseSationId);
    if (!baseStation)
    {
        return false;
    }
    baseStation->removeClient(m_tmsi);

    auto iteratorImsi = tmsiLookupImsi.find(m_tmsi);
    if (iteratorImsi == tmsiLookupImsi.end())
    {
        return false;
    }
    IMSI imsi = iteratorImsi->second;

    msisdnLookupTmsi.erase(iteratorVlr->second.msisdn);
    imsiLookupTmsi.erase(imsi);
    tmsiLookupImsi.erase(m_tmsi);

    visitorLocationRegister.erase(iteratorVlr);
    homeLocationAndEquipmentIdentityRegister->updateSubscriberStatus(imsi, SubscriberStatus::INACTIVE);
    return true; 
}

bool MobilityManagmentEntity::detachClient(M_TMSI m_tmsi)
{
    std::lock_guard<std::mutex> lock(mmeMutex);
    return detachClientWithoutMutex(m_tmsi);
}

void MobilityManagmentEntity::startHandover(M_TMSI m_tmsi, BaseStationId newBaseStationId)
{
    std::lock_guard<std::mutex> lock(mmeMutex);

    auto itVlr = visitorLocationRegister.find(m_tmsi);
    if (itVlr == visitorLocationRegister.end())
    {
        return;
    }

    BaseStationId oldBaseStationId = itVlr->second.currentBaseSationId;
    if (oldBaseStationId == newBaseStationId)
    {
        return;
    }

    BaseStation* oldBaseStation = getBaseStationByIdWithoutMutex(oldBaseStationId);
    BaseStation* newBaseStation = getBaseStationByIdWithoutMutex(newBaseStationId);

    if (!oldBaseStation || !newBaseStation)
    {
        return;
    }
    auto clientContext = oldBaseStation->releaseClientContext(m_tmsi);
    if (clientContext)
    {
        newBaseStation->acceptHandoverContext(std::move(clientContext));
        itVlr->second.currentBaseSationId = newBaseStationId;
        
        Logger::info(std::format("MME: handover successful for client with M_TMSI = {} "
                                "(old base station ID = {} -> new base station ID = {})",
                                    m_tmsi, oldBaseStationId, newBaseStationId));
    }
}

SmsResult MobilityManagmentEntity::handleSmsRequest(const SmsSendRequest& request)
{
    std::lock_guard<std::mutex> lock(mmeMutex);

    auto itSender = visitorLocationRegister.find(request.senderTmsi);
    if (itSender == visitorLocationRegister.end())
    {
        Logger::warning("MME: The sender " + request.receiverMsisdn + "is not registered in the VLR, "
                    "so it is impossible to deliver SMS");
        return std::string("Sender not registered in VLR");
    }

    auto receiverHlr = homeLocationAndEquipmentIdentityRegister->getSubscriberByMsisdn(request.receiverMsisdn);
    if (!receiverHlr.has_value())
    {
        Logger::warning("MME: SMS rejected because the receiver MSISDN unknown in HLR and EIR");
        return std::string("Receiver MSISDN unknown in HLR and EIR");
    }

    auto globalSmsIds = smsCenter->storeMessage(request.senderTmsi, itSender->second.msisdn,
                                                request.receiverMsisdn, request.text);
    if (!globalSmsIds.has_value())
    {
        return std::string("Failed to store message in SMSC because SMSC buffer is full");
    }

    auto routingInfo = findRoutingInfoForSms(request.receiverMsisdn);
    if (!routingInfo.has_value())
    {
        Logger::info("MME: receiver " + request.receiverMsisdn + " offline. "
                        "SMS stored in SMSC");
        return globalSmsIds.value();
    }

    // Immediate delivery attempt
    auto itReceiverTmsi = msisdnLookupTmsi.find(request.receiverMsisdn);
    if (itReceiverTmsi == msisdnLookupTmsi.end())
    {
        return globalSmsIds.value();
    }
    auto itReceiverVlr = visitorLocationRegister.find(itReceiverTmsi->second);
    if (itReceiverVlr == visitorLocationRegister.end())
    {
        return globalSmsIds.value();
    }

    Protocol receiverProtocol = itReceiverVlr->second.protocol;
    auto receiverSerializer = SerializerFactory::createSerializer(receiverProtocol);

    size_t offset = 0;
    size_t maxTextLength = smsCenter->getMaxTextLength();
    std::string partText;
    for (const auto& smsId : globalSmsIds.value())
    {
        partText = request.text.substr(offset, maxTextLength);
        offset += maxTextLength;

        SmsIncomingMessage incomingMessage{itReceiverVlr->first,
                                           itSender->second.msisdn,
                                           smsId,
                                           partText};

        BaseStation* baseStation = getBaseStationByIdWithoutMutex(routingInfo.value().second);
        if (!baseStation)
        {
            Logger::warning("MME: error when finding the receiver's "+ request.receiverMsisdn + " base station");
            return globalSmsIds.value();
        }

        NetworkDataPacket packetForBuffer{static_cast<uint8_t>(receiverProtocol),
                                       PacketType::SMS_RECEIVE,
                                       receiverSerializer->serialize(incomingMessage)};
        baseStation->pushMessageToBuffer(routingInfo.value().first, packetForBuffer);
        Logger::info("MME: SMS with ID = " + std::to_string(smsId)
                    + " routed immediately to the base station " + std::to_string(routingInfo.value().second));
    }

    return globalSmsIds.value();
}

void MobilityManagmentEntity::handleSmsDeliveryAcknowledge(const SmsDeliveryAcknowledge& message)
{
    std::lock_guard<std::mutex> lock(mmeMutex);

    auto smsRecord = smsCenter->getSmsRecordById(message.smsId);
    if (!smsRecord.has_value())
    {
        return;
    }

    auto itSender = visitorLocationRegister.find(smsRecord->senderTmsi);
    bool isSenderOnline = false;
    if (itSender != visitorLocationRegister.end())
    {
        isSenderOnline = true;
    }

    AcknowledgeResult result = smsCenter->processDeliveryAcknowledge(message.smsId, isSenderOnline);
    if (!result.isFound || !isSenderOnline)
    {
        return;
    }

    Logger::debug("MME: SMS with ID = " + std::to_string(message.smsId)
                  + " marked as waiting delivery report in the SMS center");
    BaseStation* baseStation = getBaseStationByTmsiWithoutMutex(smsRecord.value().senderTmsi);
    if (baseStation)
    {
        Protocol senderProtocol = itSender->second.protocol;
        auto senderSerializer = SerializerFactory::createSerializer(senderProtocol);

        SmsStatusReport smsStatusReport{smsRecord.value().senderTmsi, message.smsId, message.status};
        NetworkDataPacket packetForBuffer{static_cast<uint8_t>(senderProtocol),
                                        PacketType::SMS_STATUS,
                                        senderSerializer->serialize(smsStatusReport)};
        if (baseStation->pushMessageToBuffer(smsRecord.value().senderTmsi, packetForBuffer))
        {
            Logger::info("MME: Status report for SMS with ID = " + std::to_string(message.smsId)
            + " sent to the sender with M_TMSI = " + std::to_string(smsRecord->senderTmsi));
        }
    }
}

std::optional<std::pair<M_TMSI, BaseStationId>> MobilityManagmentEntity::findRoutingInfoForSms(const MSISDN& receiverMsisdn)
{
    auto itTmsi = msisdnLookupTmsi.find(receiverMsisdn);
    if (itTmsi == msisdnLookupTmsi.end())
    {
        return std::nullopt;
    }

    auto itVlr = visitorLocationRegister.find(itTmsi->second);
    if (itVlr == visitorLocationRegister.end())
    {
        return std::nullopt;
    }

    return std::make_pair(itTmsi->second, itVlr->second.currentBaseSationId);
}