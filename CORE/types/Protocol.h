#pragma once
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <iostream>
#include <cstdint>

enum class Protocol : uint8_t
{
    JSON = 1,
    BINARY = 2
};
std::ostream& operator<<(std::ostream& out, const Protocol& protocol);

#endif