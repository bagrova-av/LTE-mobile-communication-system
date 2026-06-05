#pragma once
#ifndef SERIALIZER_INTERFACE_H
#define SERIALIZER_INTERFACE_H

#include <vector>
#include <optional>

#include "../types.h"

class SerializerInterface
{
public:
    virtual ~SerializerInterface() = default;

    virtual BytesBuffer serialize(const LocationPoint3D& point) const = 0;
    virtual std::optional<LocationPoint3D> deserialize(const BytesBuffer& data) const = 0;
};

#endif