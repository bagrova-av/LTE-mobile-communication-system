#pragma once
#ifndef SMS_CENTER_H
#define SMS_CENTER_H

#include <chrono>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <functional>

#include "../../CORE/types.h"
#include "../../CORE/entitiesConfigs/SmsCenterConfig.h"

enum class SmsStatus
{
    PENDING,
    DELIVERED,
    WAITING_DELIVERY_REPORT,
    WAITING_FAILURE_REPORT,
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

struct PendingEvents
{
    std::vector<SmsRecord> incomingMessages;
    std::vector<SmsRecord> statusReports;
};

struct AcknowledgeResult
{
    bool isFound;
    std::optional<M_TMSI> senderTmsi;
    bool senderOnline;
};

class SmsCenter
{
private:
    SmsCenterConfig config;
    std::unordered_map<SmsId, SmsRecord> smsStorage;
    std::atomic<SmsId> nextSmsId;
    std::mutex smscMutex;

    SmsId storeOneMessageWithoutMutex(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                     const MSISDN& receiverMsisdn, const std::string& text);

public:
    SmsCenter(const SmsCenterConfig& config);

    size_t getMaxTextLength() const;
    size_t getMaxBufferCount() const;
    std::chrono::milliseconds getSmsTtlMs() const;
    std::optional<SmsRecord> getSmsRecordById(SmsId smsId);

    std::optional<std::vector<SmsId>> storeMessage(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                                const MSISDN& receiverMsisdn, const std::string& fullText);            
    std::vector<SmsRecord> getMessagesForReceiver(const MSISDN& receiverMmsisdn);
    PendingEvents popEventsForSubscriber(const MSISDN& msisdn);
    AcknowledgeResult processDeliveryAcknowledge(SmsId smsId, bool isSenderOnline);
    void processExpiredMessages();
    std::optional<SmsRecord> markSmsAsDelivered(SmsId smsId);
};

inline size_t SmsCenter::getMaxTextLength() const
{
    return config.maxTextLength;
}

inline size_t SmsCenter::getMaxBufferCount() const
{
    return config.maxBufferCount;
}

inline std::chrono::milliseconds SmsCenter::getSmsTtlMs() const
{
    return config.smsTtlMs;
}

#endif