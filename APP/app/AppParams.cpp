#include "AppParams.h"
#include "../../COMMON/logging/Logger.h"

AppConfig::AppConfig(const NetworkAddress& addr, 
                    const IMSI& imsi, const IMEI& imei,
                    const MSISDN& msisdn, const Nodes& nodes) :
    netAddr(addr),
    imsi(imsi),
    imei(imei),
    msisdn(msisdn),
    nodes(nodes)
{
    Logger::debug("AppConfig constructor with parameters called");
}

AppConfig::AppConfig(const NetworkAddress& addr, 
                    const IMSI& imsi, const IMEI& imei,
                    const MSISDN& msisdn) :
    netAddr(addr),
    imsi(imsi),
    imei(imei),
    msisdn(msisdn),
    nodes()
{
    Logger::debug("AppConfig constructor with parameters called");
}

AppConfig::AppConfig(const AppConfig& other) :
    netAddr(other.netAddr),
    imsi(other.imsi),
    imei(other.imei),
    msisdn(other.msisdn),
    nodes(other.nodes)
{
    Logger::debug("AppConfig: copy constructor called");
}

AppConfig::AppConfig(AppConfig&& other) noexcept :
    netAddr(std::move(other.netAddr)),
    imsi(std::move(other.imsi)),
    imei(std::move(other.imei)),
    msisdn(std::move(other.msisdn)),
    nodes(std::move(other.nodes))
{
    Logger::debug("AppConfig: move constructor called");
}

AppConfig& AppConfig::operator=(const AppConfig& other)
{
    Logger::debug("AppConfig: copy assignment called");
    if (this != &other)
    {
        netAddr = other.netAddr;
        imsi = other.imsi;
        imei = other.imei;
        msisdn = other.msisdn;
        nodes = other.nodes;
    }
    return *this;
}

AppConfig& AppConfig::operator=(AppConfig&& other) noexcept
{
    Logger::debug("AppConfig: move assignment called");
    if (this != &other)
    {
        netAddr = std::move(other.netAddr);
        imsi = std::move(other.imsi);
        imei = std::move(other.imei);
        msisdn = std::move(other.msisdn);
        nodes = std::move(other.nodes);
    }
    return *this;
}