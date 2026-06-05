#include "App.h"
#include "AppProtocol.h"
#include "../../COMMON/logging/Logger.h"
#include "../../CORE/network/JsonSerializer.h"
#include "../../CORE/network/BinarySerializer.h"
#include "../../CORE/ClientSocket.h"

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

void App::sendLocationToServerAsync(const LocationPoint3D& location, Protocol protocol)
{
    if(!networkTasks.push(NetworkTask{location, protocol}))
    {
        Logger::warning("App is shutting down. Cannot send location to server");
    }
}

void App::networkThreadWorker()
{
    Logger::info("Client network thread started");
    std::string response = "";

    while (true)
    {
        auto networkTask = networkTasks.pop();
        if (!networkTask.has_value())
        {
            break;
        }

        std::unique_ptr<SerializerInterface> serializer;
        uint8_t protocolHeader = static_cast<uint8_t>(networkTask.value().protocol);

        if (networkTask.value().protocol == Protocol::JSON)
        {
            serializer = std::make_unique<JsonSerializer>();
        }
        else if (networkTask.value().protocol == Protocol::BINARY)
        {
            serializer = std::make_unique<BinarySerializer>();
        }
        else
        {
            Logger::warning("App: the protocol is not supported for transmission");
            continue;
        }

        std::optional<BytesBuffer> serializedData = serializer->serialize(networkTask.value().location);
        if (!serializedData.has_value())
        {
            Logger::error("Failed to serialize location for network transfer");
            continue;
        }

        ClientSocket clientSocket;
        Logger::info("Connecting to server at " +
                    convertIPv4ToString(getIP()) + ":" + std::to_string(getPort()));
        if (!clientSocket.connectToServer(getIP(), getPort()))
        {
            continue;
        }
        Logger::info("The client has successfully connected to the server");

        if (!AppProtocol::sendLocationRequest(clientSocket, protocolHeader, serializedData.value()))
        {
            Logger::error("CommandMove: failed to send data to the server");
            continue;
        }
        Logger::info("The data with the current position of the client has been sent to the server successfully");

        std::optional<CoordinatesType> distance = AppProtocol::receiveDistanceResponse(clientSocket);
        if (!distance.has_value())
        {
            continue;
        }
        response = "Distance from " + convertLocationToString (networkTask.value().location)
                   + " to the server: " + std::to_string(distance.value());
        responsesQueue.push(std::move(response));
        Logger::info("Received distance from the server. The distance from the client to the server is " +
                    std::to_string(distance.value()));
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