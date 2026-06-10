#pragma once
#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include "SerializerInterface.h"

class JsonSerializer : public SerializerInterface
{
public:
    virtual ~JsonSerializer() = default;

    virtual BytesBuffer serialize(const NetworkMessage& message) const override;
    virtual std::optional<NetworkMessage> deserialize(PacketType packetType,
                                                      const BytesBuffer& dataNetworkFormat) const override;
};

#endif