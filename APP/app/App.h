#pragma once
#ifndef APP_H
#define APP_H

#include <memory>
#include <string>
#include <thread>

#include "../../CORE/types.h"
#include "AppParams.h"
#include "../../COMMON/ThreadSafeQueue.h"

struct NetworkTask
{
    LocationPoint3D location;
    Protocol protocol;
};

class App
{
private:
    const AppConfig config;
    AppState state;
    ThreadSafeQueue<NetworkTask> networkTasks;
    ThreadSafeQueue<std::string> responsesQueue;
    std::thread networkThread;

    void networkThreadWorker();

public:
    App(const AppConfig& appConfig, const AppState& initialState);
    ~App();

    void setStatus(bool status);
    void setLocation(const LocationPoint3D& newLocation, const LocationFlags& flags);
    void setProtocol(Protocol protocol);

    const IPv4& getIP() const;
    Port getPort() const;
    const NetworkAddress& getNetworkAddress() const;
    const IMEI& getIMEI() const;
    const IMSI& getIMSI() const;
    const Nodes& getNodes() const;
    const LocationPoint3D& getLocation() const;
    bool getStatus() const;
    Protocol getProtocol() const;

    void sendLocationToServerAsync(const LocationPoint3D& location, Protocol protocol);
    bool processResponses();
};


inline void App::setStatus(bool status)
{
    state.isActive = status;
}

inline void App::setLocation(const LocationPoint3D& newLocation, const LocationFlags& flags)
{
    state.location.updateWithFlags(newLocation, flags);
}

inline void App::setProtocol(Protocol protocol)
{
    state.protocol = protocol;
}

inline const IPv4& App::getIP() const
{ 
    return config.netAddr.getIP(); 
}

inline Port App::getPort() const
{ 
    return config.netAddr.getPort(); 
}

inline const NetworkAddress& App::getNetworkAddress() const
{
    return config.netAddr;
}

inline const IMEI& App::getIMEI() const
{
    return config.imei;
}

inline const IMSI& App::getIMSI() const
{
    return config.imsi;
}

inline const Nodes& App::getNodes() const
{
    return config.nodes;
}

inline const LocationPoint3D& App::getLocation() const
{ 
    return state.location; 
}

inline bool App::getStatus() const
{
    return state.isActive;
}

inline Protocol App::getProtocol() const
{
    return state.protocol;
}

#endif