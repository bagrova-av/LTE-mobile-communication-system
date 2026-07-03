#pragma once
#ifndef NETWORK_ADDRESS_H
#define NETWORK_ADDRESS_H

#include <string>
#include <optional>

#include "./types/IPv4.h"
#include "./types/types.h"

class NetworkAddress
{
private:
    IPv4 ip;
    Port port;

    std::optional<IPv4> parseIP(const std::string& str);
    bool validatePortLowestOctet(Octet lowestOctet);

public:
    NetworkAddress() = default;
    NetworkAddress(int64_t ipAddress, int32_t port);
    NetworkAddress(const std::string& fullAddressStr);
    NetworkAddress(const std::string& ipAddressStr, const std::string& portStr);

    NetworkAddress(const NetworkAddress& other);
    NetworkAddress(NetworkAddress&& other) noexcept;

    NetworkAddress& operator=(const NetworkAddress& other);
    NetworkAddress& operator=(NetworkAddress&& other) noexcept;

    const IPv4& getIP() const;
    Port getPort() const;
};

inline const IPv4& NetworkAddress::getIP() const
{
    return ip;
}

inline Port NetworkAddress::getPort() const
{
    return port;
}

#endif