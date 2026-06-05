#pragma once
#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include "../types.h"
#include "SerializerInterface.h"

class JsonSerializer : public SerializerInterface
{
public:
    virtual ~JsonSerializer() = default;

    virtual BytesBuffer serialize(const LocationPoint3D& point) const;
    virtual std::optional<LocationPoint3D> deserialize(const BytesBuffer& dataNetworkFormat) const;
};

#endif