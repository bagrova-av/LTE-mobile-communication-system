#pragma once
#ifndef SUBSCRIBER_RECORD_H
#define SUBSCRIBER_RECORD_H

#include <optional>
#include <chrono>

#include "../types/types.h"

enum class SubscriberStatus
{
    ACTIVE,
    BLOCKED,
    STOLEN,
    INACTIVE
};

struct IdentitySubscriberData
{
    IMSI imsi;
    IMEI imei;
    MSISDN msisdn;
    SubscriberStatus status;
};

struct SessionSubscriberData
{
    std::optional<std::chrono::system_clock::time_point> lastActivityTime;
    std::optional<MmeId> currentMmeId;
};

struct HlrEirRecord
{
    IdentitySubscriberData identityData;
    SessionSubscriberData sessionData;
};

std::optional<SubscriberStatus> convertStringToStatus(const std::string& str);

#endif