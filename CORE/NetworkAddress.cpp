#include "NetworkAddress.h"
#include "../COMMON/logging/Logger.h"
#include "../COMMON/validationUtils.h"
#include "../COMMON/parserUtils.h"
#include "../COMMON/utils.h"

#include <format>

NetworkAddress::NetworkAddress(int64_t ipAddress, int32_t port)
{
    if (ipAddress < 0 || ipAddress > 0xFFFFFFFF)
    {
        throw std::invalid_argument("Validation error: IP address is out of IPv4 range");
    }

    Octet octet1 = (ipAddress >> 24) & 0xFF;
    Octet octet2 = (ipAddress >> 16) & 0xFF;
    Octet octet3 = (ipAddress >> 8) & 0xFF;
    Octet octet4 = ipAddress & 0xFF;
    if (!validatePortLowestOctet(octet4))
    {
        throw std::invalid_argument("");
    }
    this->ip.octets[0] = octet1;
    this->ip.octets[1] = octet2;
    this->ip.octets[2] = octet3;
    this->ip.octets[3] = octet4;

    if (!validatePort(port))
    {
        throw std::invalid_argument("");
    }
    this->port = static_cast<Port>(port);
}

NetworkAddress::NetworkAddress(const std::string& fullAddressStr)
{
    size_t separatorPos = fullAddressStr.find(':');
    if (separatorPos == std::string::npos)
    {
        throw std::invalid_argument("Address must contain ':'");
    }

    std::string ipPart = fullAddressStr.substr(0, separatorPos);
    std::string portPart = fullAddressStr.substr(separatorPos + 1);

    *this = NetworkAddress(ipPart, portPart);
}

NetworkAddress::NetworkAddress(const std::string& ipAddressStr, const std::string& portStr)
{
    std::optional<IPv4> tempIP = parseIP(ipAddressStr);
    if (!tempIP.has_value())
    {
        throw std::invalid_argument("");
    }
    if (!validatePortLowestOctet(tempIP.value().octets[3]))
    {
        throw std::invalid_argument("");
    }
    this->ip = tempIP.value();

    int32_t tempPort = parsePort(portStr);
    if (!validatePort(tempPort))
    {
        throw std::invalid_argument("");
    }
    this->port = static_cast<Port>(tempPort);
}

NetworkAddress::NetworkAddress(const NetworkAddress& other) :
    ip(other.ip),
    port(other.port)
{
    Logger::debug("NetworkAddress: copy constructor called");
}

NetworkAddress::NetworkAddress(NetworkAddress&& other) noexcept :
    ip(std::move(other.ip)),
    port(std::move(other.port))
{
    Logger::debug("NetworkAddress: move constructor called");
}

NetworkAddress& NetworkAddress::operator=(const NetworkAddress& other)
{
    Logger::debug("NetworkAddress: copy assignment called");
    if (this != &other)
    {
        ip = other.ip;
        port = other.port;
    }
    return *this;
}

NetworkAddress& NetworkAddress::operator=(NetworkAddress&& other) noexcept
{
    Logger::debug("NetworkAddress: move assignment called");
    if (this != & other)
    {
        ip = std::move(other.ip);
        port = std::move(other.port);
    }
    return *this;
}

std::optional<IPv4> NetworkAddress::parseIP(const std::string& str)
{
    IPv4 ip;
    std::string strOctet;
    size_t index = 0;
    size_t start = 0;

    while (start < str.length())
    {
        if (index >= ip.octets.size())
        {
            Logger::error("Validation error: the IP must have 4 octets with values from 0 to 255, but has more");
            return std::nullopt;
        }

        size_t end = str.find('.', start);
        if (end != std::string::npos)
        {
            strOctet = str.substr(start, end - start);
            start = end + 1;
        }
        else
        {
            strOctet = str.substr(start);
            start = str.length();
        }

        auto intOctet = convertFromString<int>(strOctet);
        if (!intOctet.has_value())
        {
            Logger::error("Validation error: the IP octets must not contain characters other than digits");
            return std::nullopt;
        }

        if (intOctet.value() < 0 || intOctet.value() > 255)
        {
            Logger::error("Validation error: the numbers in the IP octets must be from 0 to 255");
            return std::nullopt;
        }
        ip.octets[index] = static_cast<Octet>(intOctet.value());
        index++;
    }
    if (index != ip.octets.size())
    {
        Logger::error(std::format("Validation error: the IP must have 4 octets with values from 0 to 255 (got {})", 
                                    index));
        return std::nullopt;
    }

    if (!validatePortLowestOctet(ip.octets[3]))
    {
        return std::nullopt;
    }

    return ip;
}

bool NetworkAddress::validatePortLowestOctet(Octet lowestOctet)
{
    if (lowestOctet < 1 || lowestOctet >= 254)
    {
        Logger::error("Validation error: the lowest byte of the IP address must be between 1 and 253");
        return false;
    }
    return true;
}