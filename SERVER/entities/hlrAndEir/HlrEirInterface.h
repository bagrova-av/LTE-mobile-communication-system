#pragma once
#ifndef HLR_EIR_INTERFACE_H
#define HLR_EIR_INTERFACE_H

#include "../../../CORE/entitiesConfigs/SubscriberRecord.h"

class HlrEirInterface
{
protected:
    virtual HlrEirRecord* getSubscriberPointerByImsi(const IMSI& imsi) = 0;

public:
    virtual ~HlrEirInterface() = default;

    virtual bool loadRecordsFromFile(const std::string& filePath) = 0;
    virtual std::optional<HlrEirRecord> getSubscriberByImsi(const IMSI& imsi) = 0;
    virtual bool updateSubscriberStatus(const IMSI& imsi, const SubscriberStatus& newStatus) = 0;
    virtual bool updateSubscriberSessionData(const IMSI& imsi,
                                            std::optional<MmeId> mmeId,
                                            std::optional<std::chrono::system_clock::time_point> time) = 0;
};

#endif