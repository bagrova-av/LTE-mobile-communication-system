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

struct RrcResponse
{
    BaseStationId baseStationId;
};

struct RrcReject
{
    std::string reason;
};

struct AttachAccept
{
    M_TMSI m_tmsi;
};

struct AttachReject
{
    std::string reason;
};

struct DetachRequest
{
    M_TMSI m_tmsi;
};

struct ErrorResponse
{
    std::string message;
};

struct DistanceResponse
{
    CoordinatesType distance;
};

using NetworkMessage = std::variant<AttachRequest,
                                    SearchRequest,
                                    AttachAccept,
                                    AttachReject,
                                    DetachRequest,
                                    RrcResponse,
                                    RrcReject,
                                    LocationPoint3D,
                                    DistanceResponse,
                                    ErrorResponse>;

#endif