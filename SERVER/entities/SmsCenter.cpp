#include "SmsCenter.h"
#include "../../COMMON/logging/Logger.h"

SmsCenter::SmsCenter(const SmsCenterConfig& config) :
    config(config)
{ }

std::optional<std::vector<SmsId>> SmsCenter::storeMessage(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                                        const MSISDN& receiverMsisdn, const std::string& fullText)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    size_t countParts = (fullText.length() + config.maxTextLength - 1) / config.maxTextLength;
    if (smsStorage.size() + countParts >= config.maxBufferCount)
    {
        Logger::warning("SMS center: buffer full, cannot store message with " + std::to_string(countParts) + " parts");
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

std::vector<SmsRecord> SmsCenter::popMessagesForReceiver(const MSISDN& receiverMsisdn)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    std::vector<SmsRecord> result;
    for (auto it = smsStorage.begin(); it != smsStorage.end(); )
    {
        if (it->second.receiverMsisdn == receiverMsisdn)
        {
            result.push_back(it->second);  // копируем запись
            it = smsStorage.erase(it);      // удаляем элемент и получаем следующий итератор
        }
        else
        {
            ++it;
        }
    }

    return result;
}

std::vector<SmsRecord> SmsCenter::popExpiredMessages()
{
    std::lock_guard<std::mutex> lock(smscMutex);

    std::vector<SmsRecord> expiredSms;
    auto now = std::chrono::system_clock::now();

    for (auto it = smsStorage.begin(); it != smsStorage.end(); )
    {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second.createdTime);
        if (duration > config.smsTtlMs)
        {
            expiredSms.push_back(it->second);
            it = smsStorage.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return expiredSms;
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