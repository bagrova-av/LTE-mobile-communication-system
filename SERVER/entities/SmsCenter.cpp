#include "SmsCenter.h"
#include "../../COMMON/logging/Logger.h"

SmsCenter::SmsCenter(const SmsCenterConfig& config) :
    config(config),
    nextSmsId(1)
{ }

std::optional<SmsRecord> SmsCenter::getSmsRecordById(SmsId smsId)
{
    auto it = smsStorage.find(smsId);
    if (it == smsStorage.end())
    {
        return std::nullopt;
    }
    return it->second;
}

std::optional<std::vector<SmsId>> SmsCenter::storeMessage(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                                        const MSISDN& receiverMsisdn, const std::string& fullText)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    size_t countParts = (fullText.length() + config.maxTextLength - 1) / config.maxTextLength;
    if (smsStorage.size() + countParts >= config.maxBufferCount)
    {
        Logger::warning("SMS center: buffer full, cannot store message with " 
                        + std::to_string(countParts) + " parts");
        return std::nullopt;
    }

    std::vector<SmsId> generatedSmsIds;
    generatedSmsIds.reserve(countParts);
    std::string textPart;
    std::optional<SmsId> smsId;
    size_t startPosition;
    for (size_t i = 0; i < countParts; ++i)
    {
        startPosition = i * config.maxTextLength;
        textPart = fullText.substr(startPosition, config.maxTextLength);

        smsId = storeOneMessageWithoutMutex(senderTmsi, senderMsisdn, receiverMsisdn, textPart);
        generatedSmsIds.push_back(smsId.value());
    }
    return generatedSmsIds;
}

SmsId SmsCenter::storeOneMessageWithoutMutex(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                            const MSISDN& receiverMsisdn, const std::string& text)
{
    SmsId smsId = nextSmsId++;
    smsStorage.emplace(smsId, SmsRecord{smsId, senderTmsi, senderMsisdn,
                                        receiverMsisdn, text,
                                        std::chrono::system_clock::now(),
                                        SmsStatus::PENDING});
    return smsId;
}

std::vector<SmsRecord> SmsCenter::getMessagesForReceiver(const MSISDN& receiverMsisdn)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    std::vector<SmsRecord> result;
    for (auto& [id, smsRecord] : smsStorage)
    {
        if (smsRecord.receiverMsisdn == receiverMsisdn && smsRecord.status == SmsStatus::PENDING)
        {
            result.push_back(smsRecord);
        }
    }
    return result;
}

PendingEvents SmsCenter::popEventsForSubscriber(const MSISDN& msisdn)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    PendingEvents events;
    for (auto it = smsStorage.begin(); it != smsStorage.end(); )
    {
        if (it->second.receiverMsisdn == msisdn && it->second.status == SmsStatus::PENDING)
        {
            events.incomingMessages.push_back(it->second);
            ++it;
        }
        else if (it->second.senderMsisdn == msisdn && 
                  (it->second.status == SmsStatus::WAITING_DELIVERY_REPORT || 
                   it->second.status == SmsStatus::WAITING_FAILURE_REPORT))
        {
            events.statusReports.push_back(it->second);
            it = smsStorage.erase(it);
        }
        else
        {
            ++it;
        }
    }
    return events;
}

AcknowledgeResult SmsCenter::processDeliveryAcknowledge(SmsId smsId, bool isSenderOnline)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    auto it = smsStorage.find(smsId);
    if (it == smsStorage.end())
    {
        return AcknowledgeResult{false, std::nullopt, isSenderOnline};
    }

    AcknowledgeResult result{true, it->second.senderTmsi, isSenderOnline};
    if (isSenderOnline)
    {
        smsStorage.erase(it);
    }
    else
    {
        it->second.status = SmsStatus::WAITING_DELIVERY_REPORT;
    }
    return result;
}

void SmsCenter::processExpiredMessages()
{
    std::lock_guard<std::mutex> lock(smscMutex);
    
    auto now = std::chrono::system_clock::now();

    for (auto& [id, record] : smsStorage)
    {
        if (record.status == SmsStatus::PENDING)
        {
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - record.createdTime);
            if (duration > config.smsTtlMs)
            {
                record.status = SmsStatus::WAITING_FAILURE_REPORT;
            }
        }
    }
}

void SmsCenter::processExpiredMessages(const std::function<bool(M_TMSI)>& isSenderOnline)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    auto now = std::chrono::system_clock::now();

    for (auto it = smsStorage.begin(); it != smsStorage.end(); )
    {
        auto& record = it->second;
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.createdTime);
        if (record.status == SmsStatus::PENDING && duration > config.smsTtlMs)
        {
            if (isSenderOnline(record.senderTmsi))
            {
                record.status = SmsStatus::WAITING_FAILURE_REPORT;
                ++it; 
            }
            else
            {
                record.status = SmsStatus::WAITING_FAILURE_REPORT;
                ++it;
            }
            // expiredSms.push_back(record);
            // it = smsStorage.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

std::optional<SmsRecord> SmsCenter::markSmsAsDelivered(SmsId smsId)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    auto it = smsStorage.find(smsId);
    if (it == smsStorage.end())
    {
        return std::nullopt;
    }

    it->second.status = SmsStatus::DELIVERED;
    SmsRecord copySms = it->second;
    smsStorage.erase(it);
    return copySms;
}