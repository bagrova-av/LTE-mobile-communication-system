#include "App.h"
#include "../../CORE/PacketTransporter.h"
#include "../../COMMON/logging/Logger.h"
#include "../../CORE/network/SerializerFactory.h"
#include "../../COMMON/constants.h"
#include "../../COMMON/parserUtils.h"

#include <iostream>

App::App(const AppConfig& appConfig, const AppState& initialState,
        const std::map<size_t, AddressBookEntry>& addressBook) :
    config(appConfig),
    state(initialState),
    addressBook(addressBook),
    nextLocalSmsId(1)
{
    Logger::info("Application initialized successfully");
    networkThread = std::thread(&App::networkThreadWorker, this);
}

App::~App()
{
    networkTasks.shutdown();

    if (networkThread.joinable())
    {
        networkThread.join();
    }
    Logger::info("Application shutdown complete");
}

std::optional<AddressBookEntry> App::getAddressBookEntryByShortcut(size_t shortcut) const
{
    auto it = addressBook.find(shortcut);
    if (it == addressBook.end())
    {
        return std::nullopt;
    }
    return it->second;
}

void App::pushTaskToQueue(TaskType type, Protocol protocol, TaskData data)
{
    if(!networkTasks.push(NetworkTask{type, protocol, std::move(data)}))
    {
        Logger::warning("App is shutting down. Cannot send task to the server");
    }
}

void App::sendLocationToServerAsync(const LocationPoint3D& location)
{
    pushTaskToQueue(TaskType::LOCATION, state.protocol, LocationTaskData{location});
}

void App::sendAttachTaskToServerAsync()
{
    pushTaskToQueue(TaskType::ATTACH, state.protocol, EmptyTaskData{});
}

void App::sendDetachTaskToServerAsync()
{
    pushTaskToQueue(TaskType::DETACH, state.protocol, EmptyTaskData{});
}

void App::sendSmsTaskToServerAsync(const SmsTaskData& smsTaskData)
{
    pushTaskToQueue(TaskType::SEND_SMS, state.protocol, smsTaskData);
}

void App::handleAttachTask(uint8_t protocolHeader,
                           const SerializerInterface& serializer)
{
    Logger::info("Connecting to server at " +
                convertIPv4ToString(getIP()) + ":" + std::to_string(getPort()));
    if (!clientSocket.connectToServer(getIP(), getPort()))
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        state.isActive = false;
        return;
    }
    Logger::info("The client has successfully connected to the server");

    LocationPoint3D location;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        location = state.location; 
    }
    SearchRequest searchRequest{config.imsi, location};
    if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                                                  protocolHeader,
                                                  PacketType::RRC_SEARCH,
                                                  serializer.serialize(searchRequest)))
    {
        clientSocket.disconnect();
        std::lock_guard<std::mutex> lock(stateMutex);
        Logger::error("App: failed to send RRC_Search request to the server");
        state.isActive = false;
        return;
    }

    bool success = false;
    std::string errorMessage = "Unknown error";

    auto rrcResponse = PacketTransporter::receiveDataPacketResponse(clientSocket);
    if (rrcResponse.has_value())
    {
        if (rrcResponse.value().packetType == PacketType::RRC_RESPONSE)
        {
            auto message = serializer.deserialize(PacketType::RRC_RESPONSE,
                                                    rrcResponse.value().data);
            if (message.has_value())
            {
                const auto& rrc = std::get<RrcResponse>(message.value());

                AttachRequest attachRequest{config.imsi, config.imei, config.msisdn, rrc.baseStationId};
                if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                                                              protocolHeader,
                                                              PacketType::ATTACH_REQUEST,
                                                              serializer.serialize(attachRequest)))
                {
                    std::lock_guard<std::mutex> lock(stateMutex);
                    Logger::error("App: failed to send attach request to the server");
                    state.isActive = false;
                    return;
                }

                auto attachResponse = PacketTransporter::receiveDataPacketResponse(clientSocket);
                if (attachResponse.has_value())
                {
                    if (attachResponse.value().packetType == PacketType::ATTACH_ACCEPT)
                    {
                        auto messageAccept = serializer.deserialize(PacketType::ATTACH_ACCEPT,
                                                                    attachResponse.value().data);
                        if (messageAccept.has_value())
                        {
                            success = true;
                            std::lock_guard<std::mutex> lock(stateMutex);
                            state.m_tmsi = std::get<AttachAccept>(messageAccept.value()).m_tmsi;
                            responsesQueue.push("Attach successful. Your M_TMSI is "
                                                + std::to_string(state.m_tmsi.value()));
                        }
                    }
                    else if (attachResponse.value().packetType == PacketType::ATTACH_REJECT)
                    {
                        errorMessage = "attach rejected by server";
                    }
                }
                else
                {
                    errorMessage = "server connection lost during registration";
                }
            }
        }
        else if (rrcResponse.value().packetType == PacketType::RRC_REJECT)
        {
            errorMessage = "network rejects the connection request: no coverage";
        }
        else if (rrcResponse.value().packetType == PacketType::ERROR_MESSAGE)
        {
            auto message = serializer.deserialize(PacketType::ERROR_MESSAGE,
                                                  rrcResponse.value().data);
            errorMessage = std::get<ErrorResponse>(message.value()).message;
        }
    }
    else
    {
        errorMessage = "server connection lost during Radio Resource Control request";
    }

    if (!success)
    {
        clientSocket.disconnect();
        std::lock_guard<std::mutex> lock(stateMutex);
        state.isActive = false;
        state.m_tmsi = std::nullopt;
        responsesQueue.push("Attach failed: " + errorMessage);
    }
}

void App::handleDetachTask(uint8_t protocolHeader, const SerializerInterface& serializer)
{
    if (state.m_tmsi.has_value())
    {
        M_TMSI m_tmsi;
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            m_tmsi = state.m_tmsi.value(); 
        }
        DetachRequest detachRequest{m_tmsi};
        if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                                                protocolHeader,
                                                PacketType::DETACH_REQUEST,
                                                serializer.serialize(detachRequest)))
        {
            clientSocket.disconnect();
            std::lock_guard<std::mutex> lock(stateMutex);
            Logger::error("App: failed to send detach request to the server");
            state.isActive = false;
            return;
        }
    }
    clientSocket.disconnect();

    std::lock_guard<std::mutex> lock(stateMutex);
    state.isActive = false;
    state.m_tmsi = std::nullopt;
    responsesQueue.push("Disconnected from network");
}

void App::handleLocationTask(uint8_t protocolHeader,
                            const SerializerInterface& serializer,
                            const NetworkTask& networkTask)
{
    LocationPoint3D location = std::get<LocationTaskData>(networkTask.data).location;
    std::optional<BytesBuffer> serializedLocation = serializer.serialize(location);
    if (!serializedLocation.has_value())
    {
        Logger::error("Failed to serialize location for network transfer");
        return;
    }

    if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                                                  protocolHeader,
                                                  PacketType::LOCATION,
                                                  serializedLocation.value()))
    {
        Logger::error("CommandMove: failed to send data to the server");
        responsesQueue.push("Network error: failed to send location");
        return;
    }
    Logger::info("The data with the current position of the client has been sent to the server successfully");

    auto distanceResponse = PacketTransporter::receiveDataPacketResponse(clientSocket);
    if (!distanceResponse.has_value())
    {
        Logger::error("Server connection lost while waiting for distance");
        responsesQueue.push("Error: server connection lost");

        clientSocket.disconnect();
        std::lock_guard<std::mutex> lock(stateMutex);
        state.isActive = false;
        return;
    }

    if (distanceResponse.value().packetType == PacketType::DISTANCE)
    {
        auto message = serializer.deserialize(PacketType::DISTANCE,
                                              distanceResponse.value().data);
        if (message.has_value())
        {
            CoordinatesType distance = std::get<DistanceResponse>(message.value()).distance;
            std::string response = "Distance from " + convertLocationToString(location)
                                    + " to the server: " + std::to_string(distance);
            responsesQueue.push(std::move(response));
            Logger::info("Received distance from the server. The distance from the client to the server is " +
                        std::to_string(distance));
        }
        else
        {
            Logger::error("Failed to deserialize distance response");
            responsesQueue.push("Received deformed distance data from server");
        }
    }
    else if (distanceResponse.value().packetType == PacketType::ERROR_MESSAGE)
    {
        auto message = serializer.deserialize(PacketType::ERROR_MESSAGE,
                                              distanceResponse.value().data);
        if (message.has_value())
        {
            std::string errorText = std::get<ErrorResponse>(message.value()).message;
            responsesQueue.push("Server error: " + errorText);
            Logger::warning("Server returned error: " + errorText);
        }
        else
        {
            responsesQueue.push("Server returned error");
            Logger::warning("Server returned error");
        }
    }
    else
    {
        Logger::warning("Received unexpected packet type during location update");
        responsesQueue.push("Error: unexpected response from server");
    }
}

void App::handleSendSmsTask(uint8_t protocolHeader,
                            const SerializerInterface& serializer,
                            const NetworkTask& networkTask)
{
    const SmsTaskData& smsData = std::get<SmsTaskData>(networkTask.data);

    M_TMSI m_tmsi;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        if (!state.m_tmsi.has_value())
        {
            Logger::error("Failed to send SMS: client is not registered (no TMSI)");
            smsStorage.updateSmsStatusByTempId(smsData.tempId, SmsDeliveryStatus::FAILED);
            return;
        }
        m_tmsi = state.m_tmsi.value();
    }

    SmsSendRequest request{m_tmsi, smsData.receiverMsisdn, smsData.tempId, smsData.text};
    if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                                                  protocolHeader,
                                                  PacketType::SMS_SEND,
                                                  serializer.serialize(request)))
    {
        Logger::error("Failed to send SMS to the server");
        smsStorage.updateSmsStatusByTempId(smsData.tempId, SmsDeliveryStatus::FAILED);
    }
}

void App::handleIncomingServerMessage(const NetworkDataPacket& dataPacket)
{
    std::optional<M_TMSI> m_tmsi;
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        m_tmsi = state.m_tmsi;
    }

    Protocol protocol = static_cast<Protocol>(dataPacket.protocolHeader);
    std::unique_ptr<SerializerInterface> serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received data packet with the unsupported protocol");
        return;
    }

    switch (dataPacket.packetType)
    {
        case PacketType::SMS_SUBMIT_ACCEPT :
        {
            Logger::debug("Client received packet of type: SMS_SUBMIT_ACCEPT");
            const auto& message = serializer->deserialize(PacketType::SMS_SUBMIT_ACCEPT,
                                                          dataPacket.data);
            if (message.has_value())
            {
                const auto& submitAccept = std::get<SmsSubmitAccept>(message.value());
                if (!linkSmsIds(submitAccept.tempId, submitAccept.globalId))
                {
                    Logger::error("Failed to associate the local ID for the message with its global value");
                }
                Logger::debug("Local ID " + std::to_string(submitAccept.tempId)
                            + " for the message associated with its global value "
                             + std::to_string(submitAccept.globalId));
            }
            else
            {
                Logger::error("Failed to deserialize SMS_SUBMIT_ACCEPT packet");
                responsesQueue.push("Received deformed network data packet");
            }
            break;
        }
        case PacketType::SMS_RECEIVE :
        {
            Logger::debug("Client received packet of type: SMS_RECEIVE");
            const auto& message = serializer->deserialize(PacketType::SMS_RECEIVE,
                                                          dataPacket.data);
            if (message.has_value())
            {
                const auto& sms = std::get<SmsIncomingMessage>(message.value());
                
                addReceivedSmsRecord(ReceivedSmsRecord{sms.senderMsisdn, sms.text});
                responsesQueue.push("New SMS from " + sms.senderMsisdn + ":\n" + sms.text);

                SmsDeliveryAcknowledge smsDeliveryAcknowledge{m_tmsi.value(),
                                                              sms.smsId,
                                                              SmsDeliveryStatus::DELIVERED};
                if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                                                        static_cast<uint8_t>(protocol),
                                                        PacketType::SMS_DELIVERY_ACKNOWLEDGE,
                                                        serializer->serialize(smsDeliveryAcknowledge)))
                {
                    Logger::error("App: failed to send sms delivery acknowledge to the server");
                    break;
                }
            }
            else
            {
                Logger::error("Failed to deserialize SMS_RECEIVE packet");
                responsesQueue.push("Received deformed network data packet");
            }
            break;
        }
        case PacketType::SMS_STATUS :
        {
            Logger::debug("Client received packet of type: SMS_STATUS");
            const auto& message = serializer->deserialize(PacketType::SMS_STATUS,
                                                          dataPacket.data);
            if (message.has_value())
            {
                const auto& report = std::get<SmsStatusReport>(message.value());
                smsStorage.updateSmsStatusByGlobalId(report.smsId, report.status);
                responsesQueue.push("Delivery SMS with ID " + std::to_string(report.smsId)
                                    + " is " + convertSmsDeliveryStatusToString(report.status));
                break;
            }
            else
            {
                Logger::error("Failed to deserialize SMS_STATUS packet");
                responsesQueue.push("Received deformed network data packet");
            }
            break;
        }
        case PacketType::ERROR_MESSAGE :
        {
            Logger::debug("Client received packet of type: ERROR_MESSAGE");
            const auto& message = serializer->deserialize(PacketType::ERROR_MESSAGE,
                                                         dataPacket.data);
            if (message.has_value())
            {
                responsesQueue.push(std::get<ErrorResponse>(message.value()).message);
                break;
            }
            else
            {
                Logger::error("Failed to deserialize ERROR_MESSAGE packet");
                responsesQueue.push("Received deformed network data packet");
            }
            break;
        }
    }
}

void App::networkThreadWorker()
{
    Logger::info("Client network thread started");

    while (true)
    {
        auto networkTask = networkTasks.tryPop();
        if (!networkTask.has_value())
        {
            if (networkTasks.isClosed())
            {
                break;
            }

            if (clientSocket.isOpen())
            {
                std::optional<NetworkDataPacket> incomingPacket;
                while ( (incomingPacket = PacketTransporter::receiveDataPacketResponse(clientSocket)).has_value() )
                {
                    handleIncomingServerMessage(incomingPacket.value());
                }
                
                if (errno == 0)
                {
                    clientSocket.disconnect();

                    std::lock_guard<std::mutex> lock(stateMutex);
                    state.isActive = false;
                    state.m_tmsi = std::nullopt;
                }
            }
            continue;
        }

        std::unique_ptr<SerializerInterface> serializer = SerializerFactory::createSerializer(
                                                                            networkTask.value().protocol);
        if (!serializer)
        {
            Logger::warning("App: the protocol is not supported for transmission");
            continue;
        }
        uint8_t protocolHeader = static_cast<uint8_t>(networkTask.value().protocol);
        switch (networkTask.value().type)
        {
            case TaskType::ATTACH :
            {
                handleAttachTask(protocolHeader, *serializer);
                break;
            }
            case TaskType::LOCATION :
            {
                if (clientSocket.isOpen())
                {
                    handleLocationTask(protocolHeader, *serializer, networkTask.value());
                }
                else
                {
                    Logger::warning("Network operation failed: no active connection to the network. "
                                    "Please, use 'ACTIVE true' first");
                }
                break;
            }
            case TaskType::SEND_SMS : 
            {
                if (clientSocket.isOpen())
                {
                    handleSendSmsTask(protocolHeader, *serializer, networkTask.value());
                }
                else
                {
                    Logger::warning("Network operation failed: no active connection to the network. "
                                    "Please, use 'ACTIVE true' first");
                }
                break;
            }
            case TaskType::DETACH :
            {
                if (clientSocket.isOpen())
                {
                    handleDetachTask(protocolHeader, *serializer);
                }
                else
                {
                    Logger::warning("Network operation failed: no active connection to the network. "
                                    "Please, use 'ACTIVE true' first");
                }
                break;
            }
        }
    }
    Logger::info("Client network thread stopped");
}

bool App::processResponses()
{
    bool queueHasResponses = false;

    while (true)
    {
        auto response = responsesQueue.tryPop();
        if (!response.has_value())
        {
            break;
        }
        std::cout << '\n' << "[Server response] " << response.value() << '\n';
        queueHasResponses = true;
    }
    return queueHasResponses;
}