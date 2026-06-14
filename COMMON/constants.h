#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>
#include <cstddef>

inline constexpr int32_t MIN_ALLOWED_PORT_NUMBER = 1024;
inline constexpr int32_t MAX_ALLOWED_PORT_NUMBER = 65'535;
inline constexpr size_t MAX_LENGTH_OF_IMSI_IMEI_MSISDN = 15;
inline constexpr size_t MIN_LENGTH_OF_IMSI = 6;
inline constexpr size_t MIN_LENGTH_OF_MSISDN = 11;

inline constexpr float MINIMAL_SIGNAL_STRENGTH = 0.0f;

inline constexpr uint8_t SUCCESS_STATUS = 1;

#endif