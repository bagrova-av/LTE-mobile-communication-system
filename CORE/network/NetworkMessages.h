#pragma once
#ifndef NETWORK_MESSAGES_H
#define NETWORK_MESSAGES_H

#include <variant>

#include "../types.h"

struct AttachRequest
{
    IMSI imsi;
    IMEI imei;
    MSISDN msisdn;
    BaseStationId baseStationId;
};

struct SearchRequest
{
    IMSI imsi;
    LocationPoint3D position;
};

using NetworkMessage = std::variant<AttachRequest, SearchRequest>;

#endif