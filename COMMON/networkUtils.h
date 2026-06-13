#pragma once
#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <arpa/inet.h>
#include <bit>
#include <optional>
#include <cstring>

#include "../CORE/types.h"

template<typename T>
concept NetworkSerializable = std::same_as<T, uint8_t> ||
                              std::same_as<T, uint16_t> ||
                              std::same_as<T, uint32_t> ||
                              std::same_as<T, uint64_t> ||
                              std::same_as<T, int8_t> ||
                              std::same_as<T, int16_t> ||
                              std::same_as<T, int32_t> ||
                              std::same_as<T, int64_t> ||
                              std::same_as<T, bool> ||
                              std::same_as<T, float> ||
                              std::same_as<T, double>;

template<typename T>
inline constexpr bool alwaysFalse = false;

template<typename T>
using NetworkBitsType_t = std::conditional_t<sizeof(T) == 1, uint8_t,
                            std::conditional_t<sizeof(T) == 2, uint16_t,
                                std::conditional_t<sizeof(T) == 4, uint32_t,
                                    std::conditional_t<sizeof(T) == 8, uint64_t, void>>>>;

template<NetworkSerializable T>
NetworkBitsType_t<T> convertHostToNetworkOrder(T value)
{
    using NetworkType = NetworkBitsType_t<T>;
    NetworkType rawBits = std::bit_cast<NetworkType>(value);

    if constexpr (sizeof(NetworkType) == 4)
    {   
        return htonl(rawBits);
    }
    else if constexpr (sizeof(NetworkType) == 2)
    {
        return htons(rawBits);
    }
    else if constexpr (sizeof(NetworkType) == 8)
    {
        if (std::endian::native == std::endian::little)
        {
            uint32_t highBits = static_cast<uint32_t>(rawBits >> 32);
            uint32_t lowBits = static_cast<uint32_t>(rawBits);
            
            highBits = htonl(highBits);
            lowBits = htonl(lowBits);
            rawBits = (static_cast<uint64_t>(lowBits) << 32) | highBits;
        }
        return rawBits;
    }
    else if constexpr (sizeof(NetworkType) == 1) 
    {
        return std::bit_cast<NetworkType>(value);
    }
    else
    {
        static_assert(alwaysFalse<NetworkType>, "Unsupported type size for convert host to network format");
    }
}

template <NetworkSerializable T>
T convertNetworkToHostOrder(NetworkBitsType_t<T> networkValue)
{
    using NetworkType = NetworkBitsType_t<T>;
    if constexpr (sizeof(NetworkType) == 4)
    {
        NetworkType rawBits = ntohl(networkValue);
        return std::bit_cast<T>(rawBits);
    }
    else if constexpr (sizeof(NetworkType) == 2)
    {
        NetworkType rawBits = ntohs(networkValue);
        return std::bit_cast<T>(rawBits);
    }
    else if constexpr (sizeof(NetworkType) == 8)
    {
        NetworkType rawBits = networkValue;

        if (std::endian::native == std::endian::little)
        {
            uint32_t highBits = static_cast<uint32_t>(rawBits >> 32);
            uint32_t lowBits = static_cast<uint32_t>(rawBits);

            highBits = ntohl(highBits);
            lowBits = ntohl(lowBits);
            rawBits = (static_cast<uint64_t>(lowBits) << 32) | highBits;
        }
        return std::bit_cast<T>(rawBits);
    }
    else if constexpr (sizeof(NetworkType) == 1) 
    {
        return std::bit_cast<T>(networkValue);
    }
    else
    {
        static_assert(alwaysFalse<NetworkType>, "Unsupported type size for convert network to host format");
    }
}

template<typename T>
void writeValue(BytesBuffer& buffer, T value, size_t& offset)
{
    auto networkValue = convertHostToNetworkOrder(value);
    size_t networkValueSize = sizeof(networkValue);
    std::memcpy(buffer.data() + offset, &networkValue, networkValueSize);
    offset += networkValueSize;
}

void writeString(BytesBuffer& buffer, const std::string& str, size_t& offset);

template<NetworkSerializable T>
std::optional<T> readValue(const BytesBuffer& buffer, size_t& offset)
{
    NetworkBitsType_t<T> networkValue;
    size_t networkValueSize = sizeof(networkValue);
    if (offset + networkValueSize > buffer.size())
    {
        return std::nullopt;
    }

    std::memcpy(&networkValue, buffer.data() + offset, networkValueSize);
    offset += networkValueSize;

    return convertNetworkToHostOrder<T>(networkValue);
}

std::optional<std::string> readString(const BytesBuffer& buffer, size_t& offset);

#endif