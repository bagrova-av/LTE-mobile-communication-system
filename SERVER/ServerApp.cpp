#include "ServerApp.h"
#include "../COMMON/utils.h"
#include "../COMMON/parserUtils.h"
#include "../CORE/network/SerializerFactory.h"
#include "../CORE/PacketTransporter.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <format>

ServerApp::ServerApp(uint16_t port, const LocationPoint3D& location, const EpcConfig& epcConfig) :
    listeningPort(port),
    location(location),
    epcConfig(epcConfig),
    serverSocket()
{
    std::vector<std::unique_ptr<BaseStation>> baseStations;
    std::vector<BaseStationConfig> baseStationsConfigs;
    if (!parseBaseStationsJSON(epcConfig.nodesConfigFile, baseStationsConfigs))
    {
        throw std::runtime_error("ServerApp: failed to load base stations from their config file");
    }
    for (const auto& baseStationConfig : baseStationsConfigs)
    {
        baseStations.emplace_back(std::make_unique<BaseStation>(baseStationConfig));
        Logger::info(std::format("BaseStation loaded: ID = {}, Position = [{};{};{}], Radius = {}", 
                                baseStationConfig.id, baseStationConfig.position.x,
                                baseStationConfig.position.y, baseStationConfig.position.z,
                                baseStationConfig.coverageRadius));
    }

    if (toLowerCase(epcConfig.hlrEir.format) != "json")
    {
        throw std::runtime_error("ServerApp: an unsupported format has been introduced for HLR and EIR");
    }
    auto hlrEir = std::make_unique<JsonHlrEir>();
    if (!hlrEir->loadRecordsFromFile(epcConfig.hlrEir.path))
    {
        throw std::runtime_error("ServerApp: failed to load HLR(EIR)");
    }

    std::vector<MmeConfig> mobilityManagmentEntitiesConfigs;
    if (!parseMmeJSON(epcConfig.mmeConfigFile, mobilityManagmentEntitiesConfigs))
    {
        throw std::runtime_error("ServerApp: failed to load mobility management entities from their config file");
    }

    SmsCenterConfig smsCenterConfig;
    if (!parseSmsCenterJSON(epcConfig.smsCenterConfigFile, smsCenterConfig))
    {
        throw std::runtime_error("ServerApp: failed to load sms center from its config file");
    }

    mme = std::make_unique<MobilityManagmentEntity>(mobilityManagmentEntitiesConfigs[0],
                                                    std::move(baseStations),
                                                    std::move(hlrEir),
                                                    std::make_unique<SmsCenter>(smsCenterConfig));

    Logger::info("ServerApp initialized successfully. Listening on port " 
                + std::to_string(listeningPort));
}

void ServerApp::networkThreadWorker(std::promise<bool> startPromise)
{
    if (!serverSocket.startListening(listeningPort))
    {
        isRunning = false;
        startPromise.set_value(false);
        return;
    }
    startPromise.set_value(true);

    while(isRunning)
    {
        int clientFileDescriptor = serverSocket.acceptClientConnection();
        if (clientFileDescriptor == -1)
        {
            if (!isRunning)
            {
                Logger::info("Network thread detected shutdown signal");
                break;
            }
            Logger::error("ServerSocket: failed to accept client connection");
            continue;
        }
        Logger::info("New client (file descriptor = " + std::to_string(clientFileDescriptor)
                     + ") connected");

        if (activeClientsFutures.size() > MAX_ACTIVE_CLIENT_CONNECTIONS)
        {
            Logger::warning("Server overloaded: maximum number of active client connections ( "
                + std::to_string(MAX_ACTIVE_CLIENT_CONNECTIONS) + " ) has been reached. "
                + "Closing new connection");
            ServerSocket::closeClientConnection(clientFileDescriptor);
            continue;
        }

        activeClientsFutures.emplace_back(std::async(std::launch::async, &ServerApp::processSingleClient,
                                                    this, clientFileDescriptor));

        activeClientsFutures.erase(
            std::remove_if(activeClientsFutures.begin(), activeClientsFutures.end(),
                            [](const std::future<void>& clientFuture)
                            {
                                return clientFuture.wait_for(std::chrono::seconds(0)) ==
                                        std::future_status::ready;
                            }
            ),
            activeClientsFutures.end()
        );
    }

    Logger::info("Waiting for " + std::to_string(activeClientsFutures.size())
                 + " active clients to finish");
    for (auto& clientFuture : activeClientsFutures)
    {
        if (clientFuture.valid())
        {
            clientFuture.wait();
        }
    }
    Logger::info("All clients finished. Network thread shutting down");
}

void ServerApp::processSingleClient(int clientFileDescriptor)
{
    std::optional<M_TMSI> currentTmsi;
    while (true)
    {
        std::optional<ReceivedDataPacket> dataPacket = PacketTransporter::receiveDataPacketRequest(clientFileDescriptor);
        if (!dataPacket.has_value())
        {
            ServerSocket::closeClientConnection(clientFileDescriptor);
            return;
        }

        Protocol receivedProtocol = static_cast<Protocol>(dataPacket.value().protocolHeader);
        auto serializer = SerializerFactory::createSerializer(receivedProtocol);
        if (!serializer)
        {
            Logger::warning("Received unknown protocol header from client");
            ServerSocket::closeClientConnection(clientFileDescriptor);
            return;
        }

        switch (dataPacket.value().packetType)
        {
            case PacketType::RRC_SEARCH :
            {
                std::optional<NetworkMessage> message = serializer->deserialize(PacketType::RRC_SEARCH,
                                                                                dataPacket.value().data);
                if (message.has_value())
                {
                    const SearchRequest& searchRequest = std::get<SearchRequest>(message.value());
                    std::optional<BaseStationId> bestBaseStationId = mme->findBestBaseStation(searchRequest.position);
                    if (bestBaseStationId.has_value())
                    {
                        sendRrcResponse(clientFileDescriptor, receivedProtocol, bestBaseStationId.value());
                    }
                    else
                    {
                        sendRrcReject(clientFileDescriptor, receivedProtocol, "No coverage");
                    }
                }
                else
                {
                    Logger::error("Failed to deserialize RRC_SEARCH packet");
                    sendErrorResponse(clientFileDescriptor, receivedProtocol, "Deformed network data packet");
                }
                break;
            }
            case PacketType::ATTACH_REQUEST :
            {
                std::optional<NetworkMessage> message = serializer->deserialize(PacketType::ATTACH_REQUEST,
                                                                                dataPacket.value().data);
                if (message.has_value())
                {
                    const AttachRequest& attachRequest = std::get<AttachRequest>(message.value());
                    std::optional<M_TMSI> m_tmsi = mme->attachClient(attachRequest.imei,
                                                                     attachRequest.imsi,
                                                                     attachRequest.baseStationId,
                                                                     clientFileDescriptor);
                    if (m_tmsi.has_value())
                    {
                        currentTmsi = m_tmsi.value();
                        sendAttachAccept(clientFileDescriptor, receivedProtocol, m_tmsi.value());
                    }
                    else
                    {
                        sendAttachReject(clientFileDescriptor, receivedProtocol, "Registration failed");
                    }
                }
                else
                {
                    Logger::error("Failed to deserialize ATTACH_REQUEST packet");
                    sendErrorResponse(clientFileDescriptor, receivedProtocol, "Deformed network data packet");
                }
                break;
            }
            case PacketType::LOCATION :
            {
                // std::optional<NetworkMessage> message = serializer->deserialize(PacketType::LOCATION,
                //                                                                 dataPacket.value().data);
                // if (message.has_value())
                // {
                //     const LocationPoint3D& targetLocation = std::get<LocationPoint3D>(message.value());
                //     Logger::info("Get a target point: " + convertLocationToString(targetLocation));

                //     std::optional<BaseStationId> bestBaseStationId = mme->findBestBaseStation(targetLocation);
                //     BaseStation* baseStation = mme->getBaseStationById(bestBaseStationId.value());

                //     CoordinatesType distance = location.distanceTo(baseStation->getPosition());
                //     Logger::info("A calculated distance from the base station (position "
                //                 + convertLocationToString(baseStation->getPosition())
                //                 + ") to the point " + convertLocationToString(targetLocation)
                //                 + " is " + std::to_string(distance));

                //     PacketTransporter::sendDistanceResponse(clientFileDescriptor, receivedProtocol, distance);
                // }
                // break;
                std::optional<NetworkMessage> message = serializer->deserialize(PacketType::LOCATION,
                                                                                dataPacket.value().data);
                if (message.has_value())
                {
                    LocationPoint3D& targetLocation = std::get<LocationPoint3D>(message.value());
                    Logger::info("Get a target point: " + convertLocationToString(targetLocation));

                    CoordinatesType distance = location.distanceTo(targetLocation);
                    Logger::info("A calculated distance to the point " + convertLocationToString(targetLocation)
                                + " is " + std::to_string(distance));

                    sendDistanceResponce(clientFileDescriptor, receivedProtocol, distance);
                }
                else
                {
                    Logger::error("Failed to deserialize LOCATION packet");
                    sendErrorResponse(clientFileDescriptor, receivedProtocol, "Invalid location data");
                }
                break;
            }
            case PacketType::DETACH_REQUEST :
            {
                std::optional<NetworkMessage> message = serializer->deserialize(PacketType::DETACH_REQUEST,
                                                                                dataPacket.value().data);
                if (message.has_value())
                {
                    const DetachRequest& detachRequest = std::get<DetachRequest>(message.value());
                    bool isDetached = mme->detachClient(detachRequest.m_tmsi);
                    if (isDetached)
                    {
                        Logger::info("Client with M_TMSI = " + std::to_string(detachRequest.m_tmsi)
                                    + " detached successfully");
                    }
                    else
                    {
                        Logger::info("Detach failed for TMSI = "
                                    + std::to_string(detachRequest.m_tmsi));
                    }
                    currentTmsi = std::nullopt;
                    ServerSocket::closeClientConnection(clientFileDescriptor);
                    return; 
                }
                else
                {
                    Logger::error("Failed to deserialize DETACH_REQUEST packet");
                    sendErrorResponse(clientFileDescriptor, receivedProtocol, "Deformed network data packet");
                }
                break;
            }
            case PacketType::SMS_SEND :
            {
                std::optional<NetworkMessage> message = serializer->deserialize(PacketType::SMS_SEND,
                                                                                dataPacket.value().data);
                if (message.has_value())
                {
                    const SmsSendRequest& smsSendRequest = std::get<SmsSendRequest>(message.value());

                    auto globalSmsIds = mme->handleSmsRequest(smsSendRequest, *serializer);
                    if (globalSmsIds.has_value())
                    {
                        for (const auto& globalSmsId : globalSmsIds.value())
                        {
                            sendSmsSubmitAccept(clientFileDescriptor, receivedProtocol,
                                                smsSendRequest.smsId, globalSmsId);
                        }
                    }
                    else
                    {
                        sendErrorResponse(clientFileDescriptor, receivedProtocol, "The SMS was not delivered");
                    }
                }
                else
                {
                    Logger::error("Failed to deserialize SMS_SEND packet");
                    sendErrorResponse(clientFileDescriptor, receivedProtocol, "Deformed SMS packet");
                }
                break;
            }
        }
    }

    if (currentTmsi.has_value())
    {
        mme->detachClient(currentTmsi.value());
        currentTmsi = std::nullopt;
    }
    ServerSocket::closeClientConnection(clientFileDescriptor);
}

void ServerApp::sendRrcResponse(int clientFileDescriptor, Protocol protocol, BaseStationId baseStationId)
{
    RrcResponse rrcResponse{baseStationId};

    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    BytesBuffer data = serializer->serialize(rrcResponse);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::RRC_RESPONSE,
                                              data);
}

void ServerApp::sendRrcReject(int clientFileDescriptor, Protocol protocol, const std::string& reason)
{
    RrcReject rrcReject{reason};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    BytesBuffer data = serializer->serialize(rrcReject);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::RRC_REJECT,
                                              data);
}

void ServerApp::sendAttachAccept(int clientFileDescriptor, Protocol protocol, M_TMSI m_tmsi)
{
    AttachAccept attachAccept{m_tmsi};

    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    BytesBuffer data = serializer->serialize(attachAccept);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::ATTACH_ACCEPT,
                                              data);
}

void ServerApp::sendAttachReject(int clientFileDescriptor, Protocol protocol, const std::string& reason)
{
    AttachReject attachReject{reason};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    BytesBuffer data = serializer->serialize(attachReject);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::ATTACH_REJECT,
                                              data);
}

void ServerApp::sendDistanceResponce(int clientFileDescriptor, Protocol protocol, CoordinatesType distance)
{
    DistanceResponse distanceResponse{distance};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    BytesBuffer data = serializer->serialize(distanceResponse);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::DISTANCE,
                                              data);
}

void ServerApp::sendErrorResponse(int clientFileDescriptor, Protocol protocol, const std::string& message)
{
    ErrorResponse errorResponse{message};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    BytesBuffer data = serializer->serialize(errorResponse);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor, 
                                              static_cast<uint8_t>(protocol),
                                              PacketType::ERROR_MESSAGE,
                                              data);
}

void ServerApp::sendSmsSubmitAccept(int clientFileDescriptor, Protocol protocol, SmsId tempSmsId, SmsId globalSmsId)
{
    SmsSubmitAccept smsSubmitAccept{tempSmsId, globalSmsId};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    BytesBuffer data = serializer->serialize(smsSubmitAccept);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::SMS_SUBMIT_ACCEPT,
                                              data);
}

void ServerApp::process()
{
    isRunning = true;

    std::promise<bool> startPromise;
    std::future<bool> startFuture = startPromise.get_future();

    std::thread networkThread(&ServerApp::networkThreadWorker, this, std::move(startPromise));

    bool serverSocketStartedSuccessfully = startFuture.get();
    if (!serverSocketStartedSuccessfully)
    {
        Logger::error("Shutting down because network thread failed to start listening");
        isRunning = false;
        if (networkThread.joinable())
        {
            networkThread.join();
        }
        return;
    }
    Logger::info("Server is running. Listening for coordinates");
    std::cout << "Server is running. ";

    std::string input;
    while (isRunning)
    {
        std::cout << '\n' << "Enter target coordinates (x [y] [z]) or 'exit': ";
        if (!std::getline(std::cin, input))
        {
            Logger::info("EOF received from console");
            isRunning = false;
            serverSocket.stopListening();
            std::cout << '\n';
            break;
        }

        std::vector<std::string> coordinates = splitCommand(input);
        if (coordinates.empty())
        {
            continue;
        }
        size_t countComponents = coordinates.size();

        if (toLowerCase(coordinates[0]) == "exit" && countComponents == 1)
        {
            Logger::info("The 'exit' command received from the local console");
            isRunning = false;
            serverSocket.stopListening();
            break;
        }
        else if (toLowerCase(coordinates[0]) == "exit")
        {
            Logger::warning("Validation error: the 'exit' command does not take any arguments");
            continue;
        }

        if (countComponents > LocationPoint3D::COUNT_COMPONENTS_IN_POINT)
        {
            Logger::warning("Validation error: expected 1 to " + std::to_string(LocationPoint3D::COUNT_COMPONENTS_IN_POINT) 
                            + " coordinates for location");
            continue;
        }

        std::vector<CoordinatesType> numericComponents;
        numericComponents.reserve(countComponents);
        bool parseSuccess = true;
        for (const auto& arg : coordinates)
        {
            auto component = convertFromString<CoordinatesType>(arg);
            if (!component.has_value())
            {
                Logger::warning("Validation error: a location coordinate '" + arg + "' contains non-numeric characters");
                parseSuccess = false;
                break;
            }
            numericComponents.push_back(component.value());
        }
        if (!parseSuccess)
        {
            continue;
        }

        LocationPoint3D targetLocation = location;
        targetLocation.updateFromVector(numericComponents);
        Logger::info("Get a target point: " + convertLocationToString(targetLocation));
        CoordinatesType distance = location.distanceTo(targetLocation);
        Logger::info("A calculated distance to the point " + convertLocationToString(targetLocation)
                     + " is " + std::to_string(distance));
        std::cout << "Target point: " << targetLocation << " => Distance: " << distance << '\n';
    }

    if (networkThread.joinable())
    {
        networkThread.join();
    }
}