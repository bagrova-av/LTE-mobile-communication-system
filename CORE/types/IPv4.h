#pragma once
#ifndef IPV4_H
#define IPV4_H

#include <cstdint>
#include <array>
#include <string>
#include <iostream> 

using Octet = uint8_t;

struct IPv4
{
    std::array<Octet, 4> octets;

    std::string convertToString() const;

    friend bool operator==(const IPv4& left, const IPv4& right)
    {
        return (left.octets[0] == right.octets[0]) && 
               (left.octets[1] == right.octets[1]) &&
               (left.octets[2] == right.octets[2]) &&
               (left.octets[3] == right.octets[3]);
    }
};
std::ostream& operator<<(std::ostream& out, const IPv4& ip);

#endif