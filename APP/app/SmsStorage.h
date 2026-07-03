#pragma once
#ifndef SMS_STORAGE_H
#define SMS_STORAGE_H

#include <mutex>
#include <optional>
#include <algorithm>
#include <set>

#include "../../CORE/types/types.h"
#include "../../CORE/network/NetworkMessages.h"

struct SentSmsRecord
{
    SmsId tempId;
    std::optional<std::vector<SmsId>> globalIds;
    MSISDN receiverMsisdn;
    std::string text;
    SmsDeliveryStatus status;
    std::set<SmsId> deliveredGlobalIds;
    size_t totalPartsExpected = 0;
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
    bool linkSmsIds(SmsId tempId, SmsId globalId, uint8_t totalParts);

    template<typename Func>
    size_t forEachSentSms(Func callback) const;
    template<typename Func>
    size_t forEachReceivedSms(Func callback) const;
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

template<typename Func>
size_t SmsStorage::forEachSentSms(Func callback) const
{
    std::lock_guard<std::mutex> lock(smsMutex);
    for (const auto& sms : sentSmsRecords)
    {
        callback(sms);
    }
    return sentSmsRecords.size();
}

template<typename Func>
size_t SmsStorage::forEachReceivedSms(Func callback) const
{
    std::lock_guard<std::mutex> lock(smsMutex);
    for (const auto& sms : receivedSmsRecords)
    {
        callback(sms);
    }
    return receivedSmsRecords.size();
}

#endif