#pragma once
#ifndef SMS_CENTER_H
#define SMS_CENTER_H

#include <chrono>
#include <mutex>

#include "../../CORE/types.h"
#include "../../CORE/entitiesConfigs/SmsCenterConfig.h"

enum class SmsStatus
{
    PENDING,
    DELIVERED,
    FAILED
};

struct SmsRecord
{
    SmsId smsId;
    M_TMSI senderTmsi;
    MSISDN senderMsisdn;
    MSISDN receiverMsisdn;
    std::string text;
    std::chrono::system_clock::time_point createdTime;
    SmsStatus status;
};

class SmsCenter
{
private:
    SmsCenterConfig config;
    std::vector<SmsRecord> smsStorage;
    SmsId nextSmsId;
    std::mutex smscMutex;

public:
    SmsCenter(const SmsCenterConfig& config);

    std::optional<SmsId> storeMessage(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                      const MSISDN& receiverMsisdn, const std::string& fullText);
    std::vector<SmsRecord> popMessagesForReceiver(const MSISDN& receiverMmsisdn);
};

#endif