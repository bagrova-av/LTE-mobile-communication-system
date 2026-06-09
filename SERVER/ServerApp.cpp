#include "ServerApp.h"
#include "../COMMON/utils.h"
#include "../COMMON/parserUtils.h"
#include "../CORE/network/JsonSerializer.h"
#include "../CORE/network/BinarySerializer.h"
#include "../CORE/PacketTransporter.h"

#include <algorithm>
#include <iostream>
#include <thread>

ServerApp::ServerApp(uint16_t port, const LocationPoint3D& location) :
    listeningPort(port),
    location(location),
    serverSocket()
{
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
    std::optional<ReceivedDataPacket> dataPacket = PacketTransporter::receiveDataPacketRequest(clientFileDescriptor);
    if (!dataPacket.has_value())
    {
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    std::unique_ptr<SerializerInterface> serializer;
    Protocol receivedProtocol = static_cast<Protocol>(dataPacket.value().protocolHeader);
    if (receivedProtocol == Protocol::JSON)
    {
        serializer = std::make_unique<JsonSerializer>();
    }
    else if (receivedProtocol == Protocol::BINARY)
    {
        serializer = std::make_unique<BinarySerializer>();
    }
    else
    {
        Logger::warning("Received unknown protocol header from client");
        ServerSocket::closeClientConnection(clientFileDescriptor);
        return;
    }

    std::optional<LocationPoint3D> targetLocation = serializer->deserialize(dataPacket.value().data);
    if (targetLocation.has_value())
    {
        Logger::info("Get a target point: " + convertLocationToString(targetLocation.value()));

        CoordinatesType distance = location.distanceTo(targetLocation.value());
        Logger::info("A calculated distance to the point " + convertLocationToString(targetLocation.value())
                    + " is " + std::to_string(distance));

        PacketTransporter::sendDistanceResponse(clientFileDescriptor, receivedProtocol, distance);
    }
    ServerSocket::closeClientConnection(clientFileDescriptor);
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