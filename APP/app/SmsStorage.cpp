#include "SmsStorage.h"
#include "../../COMMON/logging/Logger.h"

#include <algorithm>
#include <format>

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
                                                        if (id == globalId)
                                                        {
                                                            return true;
                                                        }
                                                    }
                                                    return false;
                                                }
    );

    if (!smsRecord)
    {
        return false;
    }

    if (newStatus == SmsDeliveryStatus::DELIVERED)
    {
        smsRecord->deliveredGlobalIds.insert(globalId);
        if (smsRecord->deliveredGlobalIds.size() >= smsRecord->totalPartsExpected)
        {
            if (smsRecord->status < newStatus)
            {
                smsRecord->status = newStatus;
            }
        }
    }
    else
    {
        if (smsRecord->status < newStatus)
        {
            smsRecord->status = newStatus;
        }
    }

    Logger::debug(std::format("Updating status for SMS with global ID = {} and temp ID = {}: "
                              "{}/{} parts delivered. Current status: {}",
                              globalId, smsRecord->tempId,
                              smsRecord->deliveredGlobalIds.size(), smsRecord->totalPartsExpected,
                              convertSmsDeliveryStatusToString(smsRecord->status)));
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

    if (smsRecord->status < newStatus)
    {
        smsRecord->status = newStatus;
    }
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

    if (!smsRecord->globalIds.has_value())
    {
        smsRecord->globalIds = std::vector<SmsId>{globalId};
    }
    else
    {
        smsRecord->globalIds.value().push_back(globalId);
    }

    if (smsRecord->status < SmsDeliveryStatus::IN_SMSC)
    {
        smsRecord->status = SmsDeliveryStatus::IN_SMSC;
    }
    return true;
}