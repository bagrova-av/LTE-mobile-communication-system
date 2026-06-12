#include "MobilityManagmentEntity.h"
#include "../../COMMON/constants.h"
#include "../../COMMON/logging/Logger.h"

MobilityManagmentEntity::MobilityManagmentEntity(MmeConfig config,
                                                std::vector<std::unique_ptr<BaseStation>> baseStations,
                                                std::unique_ptr<HlrEirInterface> hlrEir) :
    config(config),
    baseStations(std::move(baseStations)),
    homeLocationAndEquipmentIdentityRegister(std::move(hlrEir))
{ }

BaseStation* MobilityManagmentEntity::getBaseStationById(BaseStationId baseStationId)
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

M_TMSI MobilityManagmentEntity::generateTMSI()
{
    return nextTMSI++;
}

std::optional<BaseStationId> MobilityManagmentEntity::findBestBaseStation(const LocationPoint3D& clientLocation)
{
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

std::optional<M_TMSI> MobilityManagmentEntity::attachClient(const IMEI& imei, const IMSI& imsi,
                                                            BaseStationId baseStationId, 
                                                            int clientFileDescriptor)
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

    BaseStation* targetBaseStation = getBaseStationById(baseStationId);
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
        Logger::warning("Error: failed to register the client at the base station with id "
                        + std::to_string(baseStationId));
        return std::nullopt;
    }

    homeLocationAndEquipmentIdentityRegister->updateSubscriberSessionData(imsi, config.id,
                                                                         std::chrono::system_clock::now());
    homeLocationAndEquipmentIdentityRegister->updateSubscriberStatus(imsi, SubscriberStatus::ACTIVE);
    
    visitorLocationRegister.emplace(m_tmsi, clientsData{imei, 
                                                        subscriberRecord.value().identityData.msisdn,
                                                        baseStationId}
    );
    msisdnLookupTmsi.emplace(subscriberRecord.value().identityData.msisdn, m_tmsi);
    imsiLookupTmsi.emplace(imsi, m_tmsi);
    tmsiLookupImsi.emplace(m_tmsi,imsi);
    return m_tmsi;
}

bool MobilityManagmentEntity::detachClientWithoutMutex(M_TMSI m_tmsi)
{
    auto iteratorVlr = visitorLocationRegister.find(m_tmsi);
    if (iteratorVlr == visitorLocationRegister.end())
    {
        return false;
    }
    
    BaseStation* baseStation = getBaseStationById(iteratorVlr->second.currentBaseSationId);
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

void MobilityManagmentEntity::startHandover(M_TMSI m_tmsi, const LocationPoint3D& newClientLocation)
{
    //TO-DO
}

void MobilityManagmentEntity::sendSMStoSMSC()
{
    //TO-DO
}

std::optional<std::pair<M_TMSI, BaseStationId>> MobilityManagmentEntity::findRoutingInfoForSMS(const MSISDN& msisdn)
{
    //TO-DO
    return std::nullopt;
}