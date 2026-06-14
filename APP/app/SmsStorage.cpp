#include "SmsStorage.h"

#include <algorithm>

bool SmsStorage::updateSmsStatusByGlobalId(SmsId globalId, SmsDeliveryStatus newStatus)
{
    std::lock_guard<std::mutex> lock(smsMutex);

    auto it = std::find_if(sentSmsRecords.begin(), sentSmsRecords.end(),
                            [globalId](const SentSmsRecord& smsRecord)
                            {
                                return smsRecord.globalId == globalId;
                            }
    );

    if (it == sentSmsRecords.end())
    {
        return false;
    }
    it->status = newStatus;
    return true;
}

bool SmsStorage::updateSmsStatusByTempId(SmsId tempId, SmsDeliveryStatus newStatus)
{
    std::lock_guard<std::mutex> lock(smsMutex);

    auto it = std::find_if(sentSmsRecords.begin(), sentSmsRecords.end(),
                            [tempId](const SentSmsRecord& smsRecord)
                            {
                                return smsRecord.tempId == tempId;
                            }
    );

    if (it == sentSmsRecords.end())
    {
        return false;
    }
    it->status = newStatus;
    return true;
}

bool SmsStorage::linkSmsIds(SmsId tempId, SmsId globalId)
{
    std::lock_guard<std::mutex> lock(smsMutex);

    auto it = std::find_if(sentSmsRecords.begin(), sentSmsRecords.end(),
                            [tempId](const SentSmsRecord& smsRecord)
                            {
                                return smsRecord.tempId == tempId;
                            }
    );

    if (it == sentSmsRecords.end())
    {
        return false;
    }
    it->globalId = globalId;
    it->status = SmsDeliveryStatus::SENT_TO_SERVER;
    return true;
}