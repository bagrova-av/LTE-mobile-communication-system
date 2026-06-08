#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <cstddef>

int32_t MIN_ALLOWED_PORT_NUMBER = 1024;
int32_t MAX_ALLOWED_PORT_NUMBER = 65'535;
size_t MAX_LENGTH_OF_IMSI_AND_IMEI = 15;
size_t MIN_LENGTH_OF_IMSI = 6;

size_t MINIMAL_SIGNAL_STRENGTH = 0.0f;

#endif