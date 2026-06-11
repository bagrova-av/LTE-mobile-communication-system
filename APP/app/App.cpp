#include "App.h"
#include "../../CORE/PacketTransporter.h"
#include "../../COMMON/logging/Logger.h"
#include "../../CORE/network/SerializerFactory.h"

#include <iostream>

App::App(const AppConfig& appConfig, const AppState& initialState) :
    config(appConfig),
    state(initialState)
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

void App::pushTaskToQueue(TaskType type, Protocol protocol, TaskData data)
{
    if(!networkTasks.push(NetworkTask{type, protocol, std::move(data)}))
    {
        Logger::warning("App is shutting down. Cannot send task to the server");
    }
}

void App::sendLocationToServerAsync(const LocationPoint3D& location, Protocol protocol)
{
    pushTaskToQueue(TaskType::LOCATION, protocol, LocationTaskData{location});
}

void App::sendAttachTaskToServerAsync(Protocol protocol)
{
    pushTaskToQueue(TaskType::ATTACH, protocol, EmptyTaskData{});
}

void App::sendDetachTaskToServerAsync(Protocol protocol)
{
    pushTaskToQueue(TaskType::DETACH, protocol, EmptyTaskData{});
}

void App::handleAttachTask(const ClientSocket& clientSocket,
                           uint8_t protocolHeader,
                           const SerializerInterface& serializer)
{
    SearchRequest searchRequest{config.imsi, state.location};
    if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                                                  protocolHeader,
                                                  PacketType::RRC_SEARCH,
                                                  serializer.serialize(searchRequest)))
    {
        Logger::error("App: failed to send RRC_Search request to the server");
        return;
    }

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
                    Logger::error("App: failed to send ATTACH request to the server");
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
                            state.m_tmsi = std::get<AttachAccept>(messageAccept.value()).m_tmsi;
                            responsesQueue.push("Attach successful. Your M_TMSI is "
                                                + std::to_string(state.m_tmsi.value()));
                        }
                    }
                    else if (attachResponse.value().packetType == PacketType::ATTACH_REJECT)
                    {
                        responsesQueue.push("Attach rejected by server");
                    }
                }
            }
        }
        else if (rrcResponse.value().packetType == PacketType::RRC_REJECT)
        {
            responsesQueue.push("Network rejects the connection request: no coverage");
        }
        else if (rrcResponse.value().packetType == PacketType::ERROR_MESSAGE)
        {
            auto message = serializer.deserialize(PacketType::ERROR_MESSAGE,
                                                  rrcResponse.value().data);
            responsesQueue.push(std::get<ErrorResponse>(message.value()).message);
        }
    }
}

void App::handleLocationTask(const ClientSocket& clientSocket,
                            uint8_t protocolHeader,
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
        return;
    }
    Logger::info("The data with the current position of the client has been sent to the server successfully");

    std::optional<CoordinatesType> distance = PacketTransporter::receiveDistanceResponse(clientSocket);
    if (!distance.has_value())
    {
        return;
    }
    std::string response = "Distance from " + convertLocationToString (location)
                            + " to the server: " + std::to_string(distance.value());
    responsesQueue.push(std::move(response));
    Logger::info("Received distance from the server. The distance from the client to the server is " +
                std::to_string(distance.value()));
}

void App::networkThreadWorker()
{
    Logger::info("Client network thread started");
    // std::string response = "";

    ClientSocket clientSocket;
    Logger::info("Connecting to server at " +
                convertIPv4ToString(getIP()) + ":" + std::to_string(getPort()));
    if (!clientSocket.connectToServer(getIP(), getPort()))
    {
        return;
    }
    Logger::info("The client has successfully connected to the server");
    while (true)
    {
        auto networkTask = networkTasks.pop();
        if (!networkTask.has_value())
        {
            break;
        }

        std::unique_ptr<SerializerInterface> serializer = SerializerFactory::createSerializer(networkTask.value().protocol);
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
                // SearchRequest searchRequest{config.imsi, state.location};
                // if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                //                                               protocolHeader,
                //                                               PacketType::RRC_SEARCH,
                //                                               serializer->serialize(searchRequest)))
                // {
                //     Logger::error("App: failed to send RRC_Search request to the server");
                //     continue;
                // }

                // auto rrcResponse = PacketTransporter::receiveDataPacketResponse(clientSocket);
                // if (rrcResponse.has_value())
                // {
                //     if (rrcResponse.value().packetType == PacketType::RRC_RESPONSE)
                //     {
                //         auto message = serializer->deserialize(PacketType::RRC_RESPONSE,
                //                                                rrcResponse.value().data);
                //         if (message.has_value())
                //         {
                //             const auto& rrc = std::get<RrcResponse>(message.value());

                //             AttachRequest attachRequest{config.imsi, config.imei, config.msisdn, rrc.baseStationId};
                //             if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                //                                                           protocolHeader,
                //                                                           PacketType::ATTACH_REQUEST,
                //                                                           serializer->serialize(attachRequest)))
                //             {
                //                 Logger::error("App: failed to send ATTACH request to the server");
                //                 continue;
                //             }

                //             auto attachResponse = PacketTransporter::receiveDataPacketResponse(clientSocket);
                //             if (attachResponse.has_value())
                //             {
                //                 if (attachResponse.value().packetType == PacketType::ATTACH_ACCEPT)
                //                 {
                //                     auto messageAccept = serializer->deserialize(PacketType::ATTACH_ACCEPT,
                //                                                                  attachResponse.value().data);
                //                     if (messageAccept.has_value())
                //                     {
                //                         state.m_tmsi = std::get<AttachAccept>(messageAccept.value()).m_tmsi;
                //                         responsesQueue.push("Attach successful. Your M_TMSI is "
                //                                             + std::to_string(state.m_tmsi.value()));
                //                     }
                //                 }
                //                 else if (attachResponse.value().packetType == PacketType::ATTACH_REJECT)
                //                 {
                //                     responsesQueue.push("Attach rejected by server");
                //                 }
                //             }
                //         }
                //     }
                //     else if (rrcResponse.value().packetType == PacketType::RRC_REJECT)
                //     {
                //         responsesQueue.push("Network rejects the connection request: no coverage");
                //     }
                //     else if (rrcResponse.value().packetType == PacketType::ERROR_MESSAGE)
                //     {
                //         auto message = serializer->deserialize(PacketType::ERROR_MESSAGE,
                //                                                rrcResponse.value().data);
                //         responsesQueue.push(std::get<ErrorResponse>(message.value()).message);
                //     }
                // }
                handleAttachTask(clientSocket, protocolHeader, *serializer);
                break;
            }
            case TaskType::LOCATION :
            {
                // std::optional<BytesBuffer> serializedLocation = serializer->serialize(networkTask.value().location);
                // if (!serializedLocation.has_value())
                // {
                //     Logger::error("Failed to serialize location for network transfer");
                //     continue;
                // }
                // if (!PacketTransporter::sendDataPacketRequest(clientSocket,
                //                                               protocolHeader,
                //                                               PacketType::LOCATION,
                //                                               serializedLocation.value()))
                // {
                //     Logger::error("CommandMove: failed to send data to the server");
                //     continue;
                // }
                // Logger::info("The data with the current position of the client has been sent to the server successfully");

                // std::optional<CoordinatesType> distance = PacketTransporter::receiveDistanceResponse(clientSocket);
                // if (!distance.has_value())
                // {
                //     continue;
                // }
                // response = "Distance from " + convertLocationToString (networkTask.value().location)
                //         + " to the server: " + std::to_string(distance.value());
                // responsesQueue.push(std::move(response));
                // Logger::info("Received distance from the server. The distance from the client to the server is " +
                //             std::to_string(distance.value()));
                handleLocationTask(clientSocket, protocolHeader, *serializer, networkTask.value());
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