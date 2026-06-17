#include "ServerApp.h"
#include "../COMMON/utils.h"
#include "../COMMON/parserUtils.h"
#include "../CORE/network/SerializerFactory.h"
#include "../CORE/PacketTransporter.h"
#include "../COMMON/ConsoleProvider.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <format>

ServerApp::ServerApp(uint16_t port, const LocationPoint3D& location, const EpcConfig& epcConfig) :
    listeningPort(port),
    location(location),
    epcConfig(epcConfig),
    isRunning(true),
    activeClientsCount(0),
    lastClientDisconnectedTime(std::chrono::system_clock::now()),
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

void ServerApp::closeClientConnection(int clientFileDescriptor)
{
    ServerSocket::closeClientConnection(clientFileDescriptor);
    checkActiveClientsCount();
}

void ServerApp::checkActiveClientsCount()
{
    if (--activeClientsCount == 0)
    {
        lastClientDisconnectedTime = std::chrono::system_clock::now();
    }
}

void ServerApp::networkThreadWorker(std::promise<bool> startPromise)
{
    if (!serverSocket.startListening(listeningPort))
    {
        isRunning = false;
        isNetworkActive = false;
        startPromise.set_value(false);
        return;
    }
    startPromise.set_value(true);

    serverSocket.setAcceptTimeout(1.0); // 1 second

    while(isRunning && isNetworkActive)
    {
        int clientFileDescriptor = serverSocket.acceptClientConnection();
        if (clientFileDescriptor == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                if (activeClientsCount == 0)
                {
                    auto now = std::chrono::system_clock::now();
                    auto lastTime = lastClientDisconnectedTime.load();
                    auto idleDuration = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                    if (idleDuration >= epcConfig.simtelTtlSec)
                    {
                        Logger::info("SimTel TTL reached. Shutting down server...");
                        //isRunning = false;
                        isNetworkActive = false;
                        break;
                    }
                }
                errno = 0;
                continue;
            }
            if (!isNetworkActive)//if (!isRunning)
            {
                Logger::info("Network thread detected shutdown signal");
                break;
            }
            Logger::error("ServerSocket: failed to accept client connection");
            continue;
        }
        Logger::info("New client (file descriptor = " + std::to_string(clientFileDescriptor)
                     + ") connected");
        ++activeClientsCount;

        if (activeClientsFutures.size() > MAX_ACTIVE_CLIENT_CONNECTIONS)
        {
            Logger::warning("Server overloaded: maximum number of active client connections ( "
                + std::to_string(MAX_ACTIVE_CLIENT_CONNECTIONS) + " ) has been reached. "
                + "Closing new connection");
            ServerSocket::closeClientConnection(clientFileDescriptor);
            --activeClientsCount;
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

    if (!isRunning)
    {
        ConsoleProvider::executeSafePrint([]()
                                        {
                                            std::cout << "\n\n[Info] Network thread finished (shutdown)" << '\n';
                                        }, false
        );
    }
    else if (!isNetworkActive)
    {
        ConsoleProvider::executeSafePrint([]()
                                        {
                                            std::cout << "\n\n[System] Network kernel stopped due to TTL. "
                                                      << "Calculator is still online" << '\n';
                                        }
        );
    }
}

void ServerApp::processSingleClient(int clientFileDescriptor)
{
    std::optional<M_TMSI> currentTmsi;
    Protocol receivedProtocol = Protocol::JSON;
    while (isRunning)
    {
        auto serializer = SerializerFactory::createSerializer(receivedProtocol);
        if (!serializer)
        {
            Logger::warning("Received unknown protocol header from client");
            closeClientConnection(clientFileDescriptor);
            return;
        }

        std::optional<NetworkDataPacket> dataPacket = PacketTransporter::receiveDataPacketRequest(clientFileDescriptor);
        if (!dataPacket.has_value())
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) // recv() timed out
            {
                errno = 0;
                mme->cleanupExpiredMessages();

                sendBufferedMessages(clientFileDescriptor, currentTmsi);
                continue;
            }
            else
            {
                closeClientConnection(clientFileDescriptor);
                return;
            }
        }

        receivedProtocol = static_cast<Protocol>(dataPacket.value().protocolHeader);
        serializer = SerializerFactory::createSerializer(receivedProtocol);
        if (!serializer)
        {
            Logger::warning("Received unknown protocol header from client");
            closeClientConnection(clientFileDescriptor);
            return;
        }

        switch (dataPacket.value().packetType)
        {
            case PacketType::RRC_SEARCH :
            {
                if (!handleRrcSearch(clientFileDescriptor, receivedProtocol,
                                     *serializer, dataPacket.value().data))
                {
                    return;
                }
                break;
            }
            case PacketType::ATTACH_REQUEST :
            {
                if (!handleAttachRequest(clientFileDescriptor, receivedProtocol,
                                         *serializer, dataPacket.value().data, currentTmsi))
                {
                    return;
                }
                break;
            }
            case PacketType::LOCATION :
            {
                if (!handleLocationRequest(clientFileDescriptor, receivedProtocol,
                                           *serializer, dataPacket.value().data, currentTmsi))
                {
                    return;
                }
                break;
            }
            case PacketType::DETACH_REQUEST :
            {
                if (!handleDetachRequest(clientFileDescriptor, receivedProtocol,
                                         *serializer, dataPacket.value().data, currentTmsi))
                {
                    return;
                }
                break;
            }
            case PacketType::SMS_SEND :
            {
                if (!handleSmsSendRequest(clientFileDescriptor, receivedProtocol,
                                          *serializer, dataPacket.value().data))
                {
                    return;
                }
                break;
            }
            case PacketType::SMS_DELIVERY_ACKNOWLEDGE :
            {
                if (!handleSmsDeliveryAknowledge(clientFileDescriptor, receivedProtocol,
                                                 *serializer, dataPacket.value().data))
                {
                    return;
                }
                break;
            }
        }

        sendBufferedMessages(clientFileDescriptor, currentTmsi);
    }

    if (currentTmsi.has_value())
    {
        mme->detachClient(currentTmsi.value());
        currentTmsi = std::nullopt;
    }
    closeClientConnection(clientFileDescriptor);
}

bool ServerApp::handleRrcSearch(int clientFileDescriptor, Protocol protocol,
                                const SerializerInterface& serializer, const BytesBuffer& data)
{
    std::optional<NetworkMessage> message = serializer.deserialize(PacketType::RRC_SEARCH,
                                                                   data);
    if (message.has_value())
    {
        const SearchRequest& searchRequest = std::get<SearchRequest>(message.value());
        std::optional<BaseStationId> bestBaseStationId = mme->findBestBaseStation(searchRequest.position);
        if (bestBaseStationId.has_value())
        {
            if (!sendRrcResponse(clientFileDescriptor, protocol, bestBaseStationId.value()))
            {
                closeClientConnection(clientFileDescriptor);
                return false;
            }
        }
        else
        {
            if (!sendRrcReject(clientFileDescriptor, protocol, "No coverage"))
            {
                closeClientConnection(clientFileDescriptor);
                return false;
            }
        }
    }
    else
    {
        Logger::error("Failed to deserialize RRC_SEARCH packet");
        if (!sendErrorResponse(clientFileDescriptor, protocol, "Deformed network data packet"))
        {
            closeClientConnection(clientFileDescriptor);
            return false;
        }
    }
    return true;
}

bool ServerApp::handleAttachRequest(int clientFileDescriptor, Protocol protocol,
                                    const SerializerInterface& serializer,
                                    const BytesBuffer& data, std::optional<M_TMSI>& currentTmsi)
{
    std::optional<NetworkMessage> message = serializer.deserialize(PacketType::ATTACH_REQUEST,
                                                                   data);
    if (message.has_value())
    {
        const AttachRequest& attachRequest = std::get<AttachRequest>(message.value());
        std::optional<M_TMSI> m_tmsi = mme->attachClient(attachRequest.imei,
                                                        attachRequest.imsi,
                                                        attachRequest.baseStationId,
                                                        clientFileDescriptor,
                                                        protocol);
        if (m_tmsi.has_value())
        {
            currentTmsi = m_tmsi.value();
            if (!sendAttachAccept(clientFileDescriptor, protocol, m_tmsi.value()))
            {
                closeClientConnection(clientFileDescriptor);
                return false;
            }
        }
        else
        {
            if (!sendAttachReject(clientFileDescriptor, protocol, "Registration failed"))
            {
                closeClientConnection(clientFileDescriptor);
                return false;
            }
        }
    }
    else
    {
        Logger::error("Failed to deserialize ATTACH_REQUEST packet");
        if (!sendErrorResponse(clientFileDescriptor, protocol, "Deformed network data packet"))
        {
            closeClientConnection(clientFileDescriptor);
            return false;
        }
    }
    return true;
}

bool ServerApp::handleLocationRequest(int clientFileDescriptor, Protocol protocol,
                                      const SerializerInterface& serializer, const BytesBuffer& data,
                                      std::optional<M_TMSI>& currentTmsi)
{
    std::optional<NetworkMessage> message = serializer.deserialize(PacketType::LOCATION,
                                                                   data);
    if (message.has_value())
    {
        LocationPoint3D& targetLocation = std::get<LocationPoint3D>(message.value());
        Logger::info("Get a target point: " + convertLocationToString(targetLocation));

        auto bestBaseStationId = mme->findBestBaseStation(targetLocation);
        if (currentTmsi.has_value())
        {
            if (bestBaseStationId.has_value())
            {
                mme->startHandover(currentTmsi.value(), bestBaseStationId.value());
            }
            else
            {
                Logger::info(std::format("Client with M_TMSI {} out of coverage. Forcing detach",
                                            currentTmsi.value()));
                sendErrorResponse(clientFileDescriptor, protocol,
                                    "Network connection lost: out of coverage area");

                bool isDetached = mme->detachClient(currentTmsi.value());
                if (isDetached)
                {
                    Logger::info("Client with M_TMSI = " + std::to_string(currentTmsi.value())
                                + " detached successfully");
                }
                else
                {
                    Logger::info("Detach failed for M_TMSI = "
                                + std::to_string(currentTmsi.value()));
                }
                currentTmsi = std::nullopt;
                closeClientConnection(clientFileDescriptor);
                return false;
            }
        }

        CoordinatesType distance = location.distanceTo(targetLocation);
        Logger::info("A calculated distance to the point " + convertLocationToString(targetLocation)
                    + " is " + std::to_string(distance));

        if (!sendDistanceResponse(clientFileDescriptor, protocol, distance))
        {
            closeClientConnection(clientFileDescriptor);
            return false;
        }
    }
    else
    {
        Logger::error("Failed to deserialize LOCATION packet");
        if (!sendErrorResponse(clientFileDescriptor, protocol, "Invalid location data"))
        {
            closeClientConnection(clientFileDescriptor);
            return false;
        }
    }
    return true;
}

bool ServerApp::handleDetachRequest(int clientFileDescriptor, Protocol protocol,
                                    const SerializerInterface& serializer,
                                    const BytesBuffer& data, std::optional<M_TMSI>& currentTmsi)
{
    std::optional<NetworkMessage> message = serializer.deserialize(PacketType::DETACH_REQUEST,
                                                                                data);
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
            Logger::info("Detach failed for M_TMSI = "
                        + std::to_string(detachRequest.m_tmsi));
        }
        currentTmsi = std::nullopt;
        closeClientConnection(clientFileDescriptor);
        return false; 
    }
    else
    {
        Logger::error("Failed to deserialize DETACH_REQUEST packet");
        if (!sendErrorResponse(clientFileDescriptor, protocol, "Deformed network data packet"))
        {
            closeClientConnection(clientFileDescriptor);
            return false;
        }
    }
    return true;
}

bool ServerApp::handleSmsSendRequest(int clientFileDescriptor, Protocol protocol,
                                     const SerializerInterface& serializer,
                                     const BytesBuffer& data)
{
    std::optional<NetworkMessage> message = serializer.deserialize(PacketType::SMS_SEND,
                                                                    data);
    if (message.has_value())
    {
        const SmsSendRequest& smsSendRequest = std::get<SmsSendRequest>(message.value());

        // auto globalSmsIds = mme->handleSmsRequest(smsSendRequest);
        // if (globalSmsIds.has_value())
        // {
        //     size_t totalParts = globalSmsIds.value().size();
        //     for (const auto& globalSmsId : globalSmsIds.value())
        //     {
        //         if (!sendSmsSubmitAccept(clientFileDescriptor, protocol,
        //                                 smsSendRequest.smsId, globalSmsId, totalParts))
        //         {
        //             closeClientConnection(clientFileDescriptor);
        //             return false;
        //         }
        //     }
        // }
        auto result = mme->handleSmsRequest(smsSendRequest);
        if (std::holds_alternative<std::vector<SmsId>>(result))
        {
            const auto& globalSmsIds = std::get<std::vector<SmsId>>(result);
            size_t totalParts = globalSmsIds.size();
            for (const auto& globalSmsId : globalSmsIds)
            {
                if (!sendSmsSubmitAccept(clientFileDescriptor, protocol,
                                        smsSendRequest.smsId, globalSmsId, totalParts))
                {
                    closeClientConnection(clientFileDescriptor);
                    return false;
                }
            }
        }
        else
        {
            if (!sendSmsSubmitReject(clientFileDescriptor, protocol, smsSendRequest.smsId,
                                    "The SMS with ID = " + std::to_string(smsSendRequest.smsId)
                                    + " was not delivered (" + std::get<std::string>(result) + ")"))
            {
                closeClientConnection(clientFileDescriptor);
                return false;
            }
            // if (!sendErrorResponse(clientFileDescriptor, protocol, "SMSC is full. The SMS was not delivered"))
            // {
            //     closeClientConnection(clientFileDescriptor);
            //     return false;
            // }
        }
    }
    else
    {
        Logger::error("Failed to deserialize SMS_SEND packet");
        if (!sendErrorResponse(clientFileDescriptor, protocol, "Deformed SMS packet"))
        {
            closeClientConnection(clientFileDescriptor);
            return false;
        }
    }
    return true;
}

bool ServerApp::handleSmsDeliveryAknowledge(int clientFileDescriptor, Protocol protocol,
                                            const SerializerInterface& serializer,
                                            const BytesBuffer& data)
{
    std::optional<NetworkMessage> message = serializer.deserialize(PacketType::SMS_DELIVERY_ACKNOWLEDGE,
                                                                    data);
    if (message.has_value())
    {
        const SmsDeliveryAcknowledge& smsDeliveryAcknowledge =
                                                std::get<SmsDeliveryAcknowledge>(message.value());
        mme->handleSmsDeliveryAcknowledge(smsDeliveryAcknowledge);
    }
    else
    {
        Logger::error("Failed to deserialize SMS_DELIVERY_ACKNOWLEDGE packet");
        if (!sendErrorResponse(clientFileDescriptor, protocol, "Deformed network data packet"))
        {
            closeClientConnection(clientFileDescriptor);
            return false;
        }
    }
    return true;
}

void ServerApp::sendBufferedMessages(int clientFileDescriptor, const std::optional<M_TMSI>& currentTmsi)
{
    if (!currentTmsi.has_value())
    {
        return;
    }

    BaseStation* baseStation = mme->getBaseStationByTmsi(currentTmsi.value());
    if (!baseStation)
    {
        return;
    }

    std::optional<NetworkDataPacket> pendingPacket;
    while ( (pendingPacket = baseStation->popMessageFromBuffer(currentTmsi.value())).has_value() )
    {
        PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                                  pendingPacket.value().protocolHeader,
                                                  pendingPacket.value().packetType,
                                                  pendingPacket.value().data);
    }
}

bool ServerApp::sendRrcResponse(int clientFileDescriptor, Protocol protocol, BaseStationId baseStationId)
{
    RrcResponse rrcResponse{baseStationId};

    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(rrcResponse);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::RRC_RESPONSE,
                                              data);
    return true;
}

bool ServerApp::sendRrcReject(int clientFileDescriptor, Protocol protocol, const std::string& reason)
{
    RrcReject rrcReject{reason};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(rrcReject);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::RRC_REJECT,
                                              data);
    return true;
}

bool ServerApp::sendAttachAccept(int clientFileDescriptor, Protocol protocol, M_TMSI m_tmsi)
{
    AttachAccept attachAccept{m_tmsi};

    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(attachAccept);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::ATTACH_ACCEPT,
                                              data);
    return true;
}

bool ServerApp::sendAttachReject(int clientFileDescriptor, Protocol protocol, const std::string& reason)
{
    AttachReject attachReject{reason};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(attachReject);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::ATTACH_REJECT,
                                              data);
    return true;
}

bool ServerApp::sendDistanceResponse(int clientFileDescriptor, Protocol protocol, CoordinatesType distance)
{
    DistanceResponse distanceResponse{distance};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(distanceResponse);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::DISTANCE,
                                              data);
    return true;
}

bool ServerApp::sendErrorResponse(int clientFileDescriptor, Protocol protocol, const std::string& message)
{
    ErrorResponse errorResponse{message};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(errorResponse);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor, 
                                              static_cast<uint8_t>(protocol),
                                              PacketType::ERROR_MESSAGE,
                                              data);
    return true;
}

bool ServerApp::sendSmsSubmitAccept(int clientFileDescriptor, Protocol protocol,
                                    SmsId tempSmsId, SmsId globalSmsId, uint8_t totalParts)
{
    SmsSubmitAccept smsSubmitAccept{tempSmsId, globalSmsId, totalParts};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(smsSubmitAccept);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::SMS_SUBMIT_ACCEPT,
                                              data);
    return true;
}

bool ServerApp::sendSmsSubmitReject(int clientFileDescriptor, Protocol protocol,
                                    SmsId tempSmsId, const std::string& reason)
{
    SmsSubmitReject smsSubmitReject{tempSmsId, reason};
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(smsSubmitReject);
    PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                              static_cast<uint8_t>(protocol),
                                              PacketType::SMS_SUBMIT_REJECT,
                                              data);
    return true;
}

void ServerApp::process()
{
    isRunning = true;
    isNetworkActive = true;

    std::promise<bool> startPromise;
    std::future<bool> startFuture = startPromise.get_future();

    std::thread networkThread(&ServerApp::networkThreadWorker, this, std::move(startPromise));

    bool serverSocketStartedSuccessfully = startFuture.get();
    if (!serverSocketStartedSuccessfully)
    {
        Logger::error("Shutting down because network thread failed to start listening");
        isRunning = false;
        isNetworkActive = false;
        if (networkThread.joinable())
        {
            networkThread.join();
        }
        return;
    }
    Logger::info("Server listening for coordinates");

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
            Logger::warning("Validation error: expected 1 to "
                            + std::to_string(LocationPoint3D::COUNT_COMPONENTS_IN_POINT)
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
                Logger::warning("Validation error: a location coordinate '"
                                + arg + "' contains non-numeric characters");
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
        //std::cout << "Target point: " << targetLocation << " => Distance: " << distance << '\n';
        ConsoleProvider::executeSafePrint([&]()
                                         {
                                            std::cout << "Target point: " << targetLocation
                                                      << " => Distance: " << distance << '\n';
                                         },false
        );
    }

    isNetworkActive = false;
    if (networkThread.joinable())
    {
        networkThread.join();
    }
}