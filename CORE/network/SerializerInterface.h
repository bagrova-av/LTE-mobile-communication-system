#pragma once
#ifndef SERIALIZER_INTERFACE_H
#define SERIALIZER_INTERFACE_H

#include "NetworkMessages.h"
#include "../PacketTransporter.h"

class SerializerInterface
{
public:
    virtual ~SerializerInterface() = default;

    virtual BytesBuffer serialize(const NetworkMessage& message) const = 0;
    virtual std::optional<NetworkMessage> deserialize(PacketType packetType,
                                                      const BytesBuffer& dataNetworkFormat) const = 0;
};

#endif