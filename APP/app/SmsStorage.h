#pragma once
#ifndef SMS_STORAGE_H
#define SMS_STORAGE_H

#include <mutex>
#include <optional>
#include <algorithm>

#include "../../CORE/types.h"
#include "../../CORE/network/NetworkMessages.h"

struct SentSmsRecord
{
    SmsId tempId;
    std::optional<SmsId> globalId;
    MSISDN receiverMsisdn;
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

    template<typename Predicate>
    SentSmsRecord* findSentSmsRecord(Predicate predicate)
    {
        auto it = std::find_if(sentSmsRecords.begin(), sentSmsRecords.end(), predicate);
        if (it == sentSmsRecords.end())
        {
            return nullptr;
        }
        return &(*it);
    }

public:
    void addReceivedSms(const ReceivedSmsRecord& smsRecord);
    void addSentSms(const SentSmsRecord& smsRecord);
    bool updateSmsStatusByGlobalId(SmsId globalId, SmsDeliveryStatus newStatus);
    bool updateSmsStatusByTempId(SmsId tempId, SmsDeliveryStatus newStatus);
    bool linkSmsIds(SmsId tempId, SmsId globalId);

    std::vector<ReceivedSmsRecord> getReceivedSmsCopy() const;
    std::vector<SentSmsRecord> getSentSmsCopy() const;
};

inline void SmsStorage::addReceivedSms(const ReceivedSmsRecord& smsRecord)
{
    std::lock_guard<std::mutex> lock(smsMutex);
    receivedSmsRecords.push_back(smsRecord);
}

inline void SmsStorage::addSentSms(const SentSmsRecord& smsRecord)
{
    std::lock_guard<std::mutex> lock(smsMutex);
    sentSmsRecords.push_back(smsRecord);
}

inline std::vector<ReceivedSmsRecord> SmsStorage::getReceivedSmsCopy() const
{
    std::lock_guard<std::mutex> lock(smsMutex);
    return receivedSmsRecords;
}

inline std::vector<SentSmsRecord> SmsStorage::getSentSmsCopy() const
{
    std::lock_guard<std::mutex> lock(smsMutex);
    return sentSmsRecords;
}

#endif