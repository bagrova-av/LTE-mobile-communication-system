#include "SmsCenter.h"

SmsCenter::SmsCenter(const SmsCenterConfig& config) :
    config(config)
{ }

std::optional<std::vector<SmsId>> SmsCenter::storeMessage(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                                        const MSISDN& receiverMsisdn, const std::string& fullText)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    if (smsStorage.size() >= config.maxBufferCount)
    {
        return std::nullopt;
    }

    std::vector<SmsId> generatedSmsIds;
    size_t fullTextSize = fullText.length();
    std::string textPart;
    std::optional<SmsId> smsId;
    for (size_t i = 0; i < fullTextSize; i += config.maxTextLength)
    {
        textPart = fullText.substr(i, config.maxTextLength);
        smsId = storeOneMessage(senderTmsi, senderMsisdn, receiverMsisdn, textPart);
        if (!smsId.has_value())
        {
            return std::nullopt;
        }
        generatedSmsIds.push_back(smsId.value());
    }
    return generatedSmsIds;
}

std::optional<SmsId> SmsCenter::storeOneMessage(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                                const MSISDN& receiverMsisdn, const std::string& text)
{
    if (smsStorage.size() >= config.maxBufferCount)
    {
        return std::nullopt;
    }

    SmsId smsId = nextSmsId++;
    smsStorage.emplace(smsId, SmsRecord{smsId, senderTmsi, senderMsisdn,
                                        receiverMsisdn, text,
                                        std::chrono::system_clock::now(),
                                        SmsStatus::PENDING});
    return smsId;
}

std::vector<SmsRecord> SmsCenter::popMessagesForReceiver(const MSISDN& receiverMsisdn)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    std::vector<SmsRecord> result;
    smsStorage.erase(
        std::remove_if(smsStorage.begin(), smsStorage.end(),
                        [&](const SmsRecord& record)
                        {
                            if (record.receiverMsisdn == receiverMsisdn)
                            {
                                result.push_back(record);
                                return true;
                            }
                            return false;
                        }
        ),
        smsStorage.end()
    );

    return result;
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