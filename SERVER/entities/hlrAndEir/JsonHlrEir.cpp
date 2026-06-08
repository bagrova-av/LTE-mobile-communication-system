#include "JsonHlrEir.h"
#include "../../../COMMON/logging/Logger.h"

#include <fstream>
#include <format>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool JsonHlrEir::loadRecordsFromFile(const std::string& filePath)
{
    std::ifstream hlrEirFile(filePath);
    if (!hlrEirFile.is_open())
    {
        Logger::error("Error: could not open file with HLR and EIR records: " + filePath);
        return false;
    }

    json data;
    try
    {
        hlrEirFile >> data;
        for (const auto& item : data)
        {
            IMSI imsi = item["imsi"].get<IMSI>();
            IMEI imei = item["imei"].get<IMEI>();
            MSISDN msisdn = item["msisdn"].get<MSISDN>();
            std::string statusString = item["status"].get<std::string>();

            std::optional<SubscriberStatus> status = convertStringToStatus(statusString);
            if (!status.has_value())
            {
                Logger::error("Error: failed to process JSON file with HLR and EIR records. "
                            "Subscriber status must be 'active', 'blocked', 'stolen' or 'inactive'");
                return false;
            }

            IdentitySubscriberData identityData{imsi, imei, msisdn, status.value()};
            SessionSubscriberData sessionData{std::nullopt, std::nullopt};
            subscriberRecords.emplace(imsi, 
                                      HlrEirRecord{std::move(identityData), std::move(sessionData)}
            );
            
        }
    }
    catch (const json::exception& ex)
    {
        Logger::error(std::format("Error: failed to process JSON file with HLR and EIR records.\nDetails: {}", 
                                    ex.what()));
        return false;
    }

    hlrEirFile.close();
    return true;
}

HlrEirRecord* JsonHlrEir::getSubscriberPointerByImsi(const IMSI& imsi)
{
    auto it = subscriberRecords.find(imsi);
    if (it == subscriberRecords.end())
    {
        return nullptr;
    }
    return &(it->second);
}

std::optional<HlrEirRecord> JsonHlrEir::getSubscriberByImsi(const IMSI& imsi)
{
    HlrEirRecord* subscriberRecord = getSubscriberPointerByImsi(imsi);
    if (!subscriberRecord)
    {
        return std::nullopt;
    }
    return *subscriberRecord;
}

bool JsonHlrEir::updateSubscriberStatus(const IMSI& imsi, const SubscriberStatus& newStatus)
{
    HlrEirRecord* subscriberRecord = getSubscriberPointerByImsi(imsi);
    if (!subscriberRecord)
    {
        return false;
    }
    subscriberRecord->identityData.status = newStatus;
    return true;
}

bool JsonHlrEir::updateSubscriberSessionData(const IMSI& imsi,
                                            std::optional<MmeId> mmeId,
                                            std::optional<std::chrono::system_clock::time_point> time)
{
    HlrEirRecord* subscriberRecord = getSubscriberPointerByImsi(imsi);
    if (!subscriberRecord)
    {
        return false;
    }

    if (mmeId.has_value())
    {
        subscriberRecord->sessionData.currentMmeId = mmeId.value();
    }
    if (time.has_value())
    {
        subscriberRecord->sessionData.lastActivityTime = time.value();
    }
    return true;
}