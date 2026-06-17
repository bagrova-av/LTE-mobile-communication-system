#include "SmsStorage.h"
#include "../../COMMON/logging/Logger.h"

#include <algorithm>

bool SmsStorage::updateSmsStatusByGlobalId(SmsId globalId, SmsDeliveryStatus newStatus)
{
    std::lock_guard<std::mutex> lock(smsMutex);

    SentSmsRecord* smsRecord = findSentSmsRecord([globalId](const auto& record)
                                                {
                                                    if (!record.globalIds.has_value())
                                                    {
                                                        return false;
                                                    }

                                                    for (const auto& id : record.globalIds.value())
                                                    {
                                                        return id == globalId;
                                                    }
                                                }
    );

    if (!smsRecord)
    {
        return false;
    }

    if (newStatus == SmsDeliveryStatus::DELIVERED)
    {
        ++smsRecord->deliveredPartsCount;
        if (smsRecord->deliveredPartsCount >= smsRecord->totalPartsExpected)
        {
            smsRecord->status = SmsDeliveryStatus::DELIVERED;
        }
    }
    else
    {
        smsRecord->status = newStatus;
    }

    //smsRecord->status = newStatus;
    Logger::debug("Updating status for SMS with global ID = " + std::to_string(globalId)
                   + " to " + convertSmsDeliveryStatusToString(newStatus));
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

bool SmsStorage::linkSmsIds(SmsId tempId, SmsId globalId, uint8_t totalParts)
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

    if (smsRecord->totalPartsExpected == 0)
    {
        smsRecord->totalPartsExpected = totalParts;
    }

    if (smsRecord->globalIds.has_value())
    {
        smsRecord->globalIds.value().push_back(globalId);
    }
    else
    {
        smsRecord->globalIds = std::vector<SmsId>{globalId};
    }
    //smsRecord->globalId = globalId;
    smsRecord->status = SmsDeliveryStatus::IN_SMSC;
    return true;
}