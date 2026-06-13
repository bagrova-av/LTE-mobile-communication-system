#include "SmsCenter.h"

SmsCenter::SmsCenter(const SmsCenterConfig& config) :
    config(config)
{ }

std::optional<SmsId> SmsCenter::storeMessage(M_TMSI senderTmsi, const MSISDN& senderMsisdn,
                                            const MSISDN& receiverMsisdn, const std::string& fullText)
{
    std::lock_guard<std::mutex> lock(smscMutex);

    if (smsStorage.size() >= config.maxBufferCount)
    {
        return std::nullopt;
    }

    SmsId smsId = nextSmsId++;

    size_t fullTextSize = fullText.length();
    std::string textPart;
    for (size_t i = 0; i < fullTextSize; i += config.maxTextLength)
    {
        textPart = fullText.substr(i, config.maxTextLength);
        storeMessage(senderTmsi, senderMsisdn, receiverMsisdn, textPart);
    }

    smsStorage.emplace_back(SmsRecord{smsId, senderTmsi, senderMsisdn,
                                      receiverMsisdn, fullText,
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