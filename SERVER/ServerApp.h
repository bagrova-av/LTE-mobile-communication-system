#pragma once
#ifndef SERVER_APP_H
#define SERVER_APP_H

#include <atomic>
#include <future>

#include "../CORE/types.h"
#include "../CORE/ServerSocket.h"

class ServerApp
{
private:
    size_t MAX_ACTIVE_CLIENT_CONNECTIONS = 100;
    const uint16_t listeningPort;
    const LocationPoint3D location;
    std::atomic<bool> isRunning;
    ServerSocket serverSocket;
    std::vector<std::future<void>> activeClientsFutures;

    void networkThreadWorker(std::promise<bool> startPromise);
    void processSingleClient(int clientFileDescriptor);

public:
    ServerApp(uint16_t port, const LocationPoint3D& location);
    
    void process();

    uint16_t getPort() const;
    const LocationPoint3D& getLocation() const;
};

inline uint16_t ServerApp::getPort() const
{
    return listeningPort;
}
inline const LocationPoint3D& ServerApp::getLocation() const
{
    return location;
}

#endif