#include "SmsStorage.h"

#include <algorithm>

void SmsStorage::updateSmsStatus(SmsId globalId, SmsDeliveryStatus newStatus)
{
    std::lock_guard<std::mutex> lock(smsMutex);

    auto it = std::find_if(sentSmsRecords.begin(), sentSmsRecords.end(),
                            [globalId](const SentSmsRecord& record)
                            {
                                return record.globalId == globalId;
                            }
    );

    if (it != sentSmsRecords.end())
    {
        it->status = newStatus;
    }
}