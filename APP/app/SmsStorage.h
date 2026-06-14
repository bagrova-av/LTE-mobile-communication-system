#pragma once
#ifndef SMS_STORAGE_H
#define SMS_STORAGE_H

#include <mutex>

#include "../../CORE/types.h"

enum class SmsDeliveryStatus
{
    SENDING,
    SENT_TO_SERVER,
    DELIVERED,
    FAILED
};

struct SentSmsRecord
{
    SmsId tempId;
    SmsId globalId;
    MSISDN receiver;
    std::string text;
    SmsDeliveryStatus status;
};

struct ReceivedSmsRecord
{
    MSISDN senderMsisdn;
    std::string text;
};

class SmsStorage
{
private:
    std::vector<SentSmsRecord> sentSmsRecords;
    std::vector<ReceivedSmsRecord> receivedSmsRecords;
    mutable std::mutex smsMutex;

public:
    void addReceivedSms(const ReceivedSmsRecord& smsRecord);
    std::vector<ReceivedSmsRecord> getReceivedSmsCopy() const;
    void updateSmsStatus(SmsId globalId, SmsDeliveryStatus newStatus);
};

inline void SmsStorage::addReceivedSms(const ReceivedSmsRecord& smsRecord)
{
    std::lock_guard<std::mutex> lock(smsMutex);
    receivedSmsRecords.push_back(smsRecord);
}

inline std::vector<ReceivedSmsRecord> SmsStorage::getReceivedSmsCopy() const
{
    std::lock_guard<std::mutex> lock(smsMutex);
    return receivedSmsRecords;
}

#endif