#include "SmsStorage.h"
#include "../../COMMON/logging/Logger.h"

#include <algorithm>

bool SmsStorage::updateSmsStatusByGlobalId(SmsId globalId, SmsDeliveryStatus newStatus)
{
    std::lock_guard<std::mutex> lock(smsMutex);

    SentSmsRecord* smsRecord = findSentSmsRecord([globalId](const auto& record)
                                                {
                                                    return record.globalId == globalId;
                                                }
    );

    if (!smsRecord)
    {
        return false;
    }
    smsRecord->status = newStatus;
    Logger::debug("Updating status for SMS with global ID " + std::to_string(globalId));
    return true;
}

bool SmsStorage::updateSmsStatusByTempId(SmsId tempId, SmsDeliveryStatus newStatus)
{
    std::lock_guard<std::mutex> lock(smsMutex);

    SentSmsRecord* smsRecord = findSentSmsRecord([tempId](const auto& record)
                                                {
                                                    return record.tempId == tempId;
                                                }
    );

    if (!smsRecord)
    {
        return false;
    }
    smsRecord->status = newStatus;
    return true;
}

bool SmsStorage::linkSmsIds(SmsId tempId, SmsId globalId)
{
    std::lock_guard<std::mutex> lock(smsMutex);

    SentSmsRecord* smsRecord = findSentSmsRecord([tempId](const auto& record)
                                                {
                                                    return record.tempId == tempId;
                                                }
    );

    if (!smsRecord)
    {
        return false;
    }
    smsRecord->globalId = globalId;
    smsRecord->status = SmsDeliveryStatus::IN_SMSC;
    return true;
}