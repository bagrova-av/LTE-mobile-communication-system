#pragma once
#ifndef NETWORK_TASK_H
#define NETWORK_TASK_H

#include <variant>

#include "../../CORE/types.h"

enum class TaskType
{
    ATTACH,
    LOCATION, //legacy for compatibility
    SEND_SMS,
    HANDOVER,
    DETACH
};

struct LocationTaskData
{
    LocationPoint3D location;
};

struct SmsTaskData
{
    MSISDN receiverMsisdn;
    SmsId tempId;
    std::string text;
};

struct EmptyTaskData { };

using TaskData = std::variant<EmptyTaskData,
                              LocationTaskData,
                              SmsTaskData>;

struct NetworkTask
{
    TaskType type;
    Protocol protocol;
    TaskData data;
};

#endif