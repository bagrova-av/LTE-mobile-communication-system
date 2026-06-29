#include "types.h"

std::ostream& operator<<(std::ostream& out, const IPv4& ip)
{
    return out << static_cast<int>(ip.octets[0]) << "."
               << static_cast<int>(ip.octets[1]) << "."
               << static_cast<int>(ip.octets[2]) << "."
               << static_cast<int>(ip.octets[3]);
}

std::string IPv4::convertToString() const
{
    return std::format("{}.{}.{}.{}", octets[0],
                                      octets[1],
                                      octets[2],
                                      octets[3]);
}

std::ostream& operator<<(std::ostream& out, const Protocol& protocol)
{
    switch (protocol)
    {
        case Protocol::BINARY:
            out << "BINARY";
            break;
        case Protocol::JSON:
            out << "JSON";
            break;
        default:
            out << "UNKNOWN";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const LocationPoint3D& point)
{
    return ( out << std::fixed << std::setprecision(4)
                 << "(" << point.x << ";" << point.y << ";" << point.z << ")" );
}