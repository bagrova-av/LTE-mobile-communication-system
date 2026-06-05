#include "StartupParams.h"

#include "../../COMMON/logging/Logger.h"

StartupParams::StartupParams(const StartupParams& other) :
    ip(other.ip),
    port(other.port),
    imsi(other.imsi),
    imei(other.imei),
    initialLocation(other.initialLocation),
    nodes(other.nodes)
{
    Logger::debug("StartupParams: copy constructor called");
}

StartupParams::StartupParams(StartupParams&& other) noexcept :
    ip(std::move(other.ip)),
    port(std::move(other.port)),
    imsi(std::move(other.imsi)),
    imei(std::move(other.imei)),
    initialLocation(std::move(other.initialLocation)),
    nodes(std::move(other.nodes))
{
    Logger::debug("StartupParams: move constructor called");
}

StartupParams& StartupParams::operator=(const StartupParams& other)
{
    Logger::debug("StartupParams: copy assignment called");
    if (this != &other)
    {
        ip = other.ip;
        port = other.port;
        imsi = other.imsi;
        imei = other.imei;
        initialLocation = other.initialLocation;
        nodes = other.nodes;
    }
    return *this;
}

StartupParams& StartupParams::operator=(StartupParams&& other) noexcept
{
    Logger::debug("StartupParams: move assignment called");
    if (this != &other)
    {
        ip = std::move(other.ip);
        port = std::move(other.port);
        imsi = std::move(other.imsi);
        imei = std::move(other.imei);
        initialLocation = std::move(other.initialLocation);
        nodes = std::move(other.nodes);
    }
    return *this;
}