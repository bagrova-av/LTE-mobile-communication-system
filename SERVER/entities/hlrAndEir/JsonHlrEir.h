#pragma once
#ifndef JSON_HLR_EIR_H
#define JSON_HLR_EIR_H

#include <unordered_map>

#include "HlrEirInterface.h"

class JsonHlrEir : public HlrEirInterface
{
private:
    std::unordered_map<IMSI, HlrEirRecord> subscriberRecords;
    std::unordered_map<MSISDN, IMSI> msisdnLookupImsi;

    virtual HlrEirRecord* getSubscriberPointerByImsi(const IMSI& imsi) override;
    virtual HlrEirRecord* getSubscriberPointerByMsisdn(const MSISDN& msisdn) override;

public:
    virtual ~JsonHlrEir() = default;

    virtual bool loadRecordsFromFile(const std::string& filePath) override;
    virtual std::optional<HlrEirRecord> getSubscriberByImsi(const IMSI& imsi) override;
    virtual std::optional<HlrEirRecord> getSubscriberByMsisdn(const MSISDN& msisdn) override;
    virtual bool updateSubscriberStatus(const IMSI& imsi, const SubscriberStatus& newStatus) override;
    virtual bool updateSubscriberSessionData(const IMSI& imsi,
                                            std::optional<MmeId> mmeId,
                                            std::optional<std::chrono::system_clock::time_point> time) override;
};

#endif