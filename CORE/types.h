#pragma once
#ifndef TYPES_H
#define TYPES_H

#include <ostream>
#include <iomanip>
#include <format>

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <utility>
#include <cmath>

#include <concepts>
#include <type_traits>

using Octet = uint8_t;
using IMSI = std::string;
using M_TMSI = uint32_t;
using IMEI = std::string;
using MSISDN = std::string;
using Nodes = std::string;
using Port = uint16_t;
using BaseStationId = uint32_t;
using MmeId = size_t;
using SmsId = uint32_t;
using Seconds = long long;
using Milliseconds = long long;
using Microseconds = long long;

using BytesBuffer = std::vector<uint8_t>;

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


struct LocationFlags
{
    uint8_t updateX : 1;
    uint8_t updateY : 1;
    uint8_t updateZ : 1;
};

template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
struct Point3D
{
    using valueType = T;

    static constexpr size_t COUNT_COMPONENTS_IN_POINT = 3;
    T x = 0;
    T y = 0;
    T z = 0;

    friend bool operator==(const Point3D& left, const Point3D& right)
    {
        return (left.x == right.x) && (left.y == right.y) && (left.z == right.z);
    }

    T distanceTo(const Point3D& pointTarget) const;
    static std::pair<Point3D, LocationFlags> createUpdateFromVector(const std::vector<T>& pointComponents);
    void updateFromVector(const std::vector<T>& pointComponents);
    void updateWithFlags(const Point3D& newPoint, const LocationFlags& flags);
    std::string convertToString() const;
};

template <Numeric T>
T Point3D<T>::distanceTo(const Point3D<T>& pointTarget) const
{
    T dx = pointTarget.x - x;
    T dy = pointTarget.y - y;
    T dz = pointTarget.z - z;

    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

template <Numeric T>
std::pair<Point3D<T>, LocationFlags> Point3D<T>::createUpdateFromVector(const std::vector<T>& pointComponents)
{
    Point3D<T> newLocation;
    LocationFlags flags{0, 0, 0};
    size_t countComponents = pointComponents.size();

    if (countComponents >= 1)
    {
        flags.updateX = 1;
        newLocation.x = pointComponents[0];
    }
    if (countComponents >= 2)
    {
        flags.updateY = 1;
        newLocation.y = pointComponents[1];
    }
    if (countComponents >= 3)
    {
        flags.updateZ = 1;
        newLocation.z = pointComponents[2];
    }

    return {newLocation, flags};
}

template <Numeric T>
void Point3D<T>::updateFromVector(const std::vector<T>& pointComponents)
{
    size_t countComponents = pointComponents.size();
    if (countComponents >= 1)
    {
        x = pointComponents[0];
    }
    if (countComponents >= 2)
    {
        y = pointComponents[1];
    }
    if (countComponents >= 3)
    {
        z = pointComponents[2];
    }
}

template <Numeric T>
void Point3D<T>::updateWithFlags(const Point3D<T>& newPoint, const LocationFlags& flags)
{
    if (flags.updateX)
    {
        x = newPoint.x;
    }
    if (flags.updateY)
    {
        y = newPoint.y;
    }
    if (flags.updateZ)
    {
        z = newPoint.z;
    }
}

template <Numeric T>
std::string Point3D<T>::convertToString() const
{
    return std::format("({};{};{})", x, y, z);
}

template<Numeric T>
std::ostream& operator<<(std::ostream& out, const Point3D<T>& point)
{
    return ( out << std::fixed << std::setprecision(4)
                 << "(" << point.x << ";" << point.y << ";" << point.z << ")" );
}

using CoordinatesType = float;
using LocationPoint3D = Point3D<CoordinatesType>;
std::ostream& operator<<(std::ostream& out, const LocationPoint3D& point);

enum class Protocol : uint8_t
{
    JSON = 1,
    BINARY = 2
};
std::ostream& operator<<(std::ostream& out, const Protocol& protocol);

#endif

