#pragma once
#ifndef BINARY_SERIALIZER_H
#define BINARY_SERIALIZER_H

#include "SerializerInterface.h"

class BinarySerializer : public SerializerInterface
{
public:
    virtual ~BinarySerializer() = default;

    virtual BytesBuffer serialize(const NetworkMessage& message) const override;
    virtual std::optional<NetworkMessage> deserialize(PacketType packetType,
                                                      const BytesBuffer& dataNetworkFormat) const override;
    // virtual BytesBuffer serialize(const LocationPoint3D& point) const;
    // virtual std::optional<LocationPoint3D> deserialize(const BytesBuffer& dataNetworkFormat) const;
};

#endif