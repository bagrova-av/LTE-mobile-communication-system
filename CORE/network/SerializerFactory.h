#pragma once
#ifndef SERIALIZER_FACTORY_H
#define SERIALIZER_FACTORY_H

#include <memory>

#include "JsonSerializer.h"
#include "BinarySerializer.h"

class SerializerFactory
{
public:
    static std::unique_ptr<SerializerInterface> createSerializer(Protocol protocol)
    {
        if (protocol == Protocol::JSON)
        {
            return std::make_unique<JsonSerializer>();
        }
        else if (protocol == Protocol::BINARY)
        {
            return std::make_unique<BinarySerializer>();
        }
        return nullptr;
    }
};

#endif