#pragma once
#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <cstdint>
#include <string>

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

#endif

