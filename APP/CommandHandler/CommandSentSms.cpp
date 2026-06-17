#include "CommandSentSms.h"
#include "../../COMMON/constants.h"
#include "../../CORE/network/NetworkMessages.h"
#include "../../COMMON/logging/Logger.h"

#include <iostream>
#include <iomanip>

std::string CommandSentSms::covertIdsToString(const std::optional<std::vector<SmsId>>& ids)
{
    if (!ids.has_value() || ids.value().empty())
    {
        return std::string("-");
    }

    std::string res;
    size_t countIds = ids.value().size();
    for (size_t i = 0; i < countIds; ++i)
    {
        res += std::to_string(ids.value()[i]);
        if (i < countIds - 1)
        {
            res += ",";
        }
    }
    return res;
}

bool CommandSentSms::execute(const std::vector<std::string>& args)
{
    if (!args.empty())
    {
        Logger::warning("Error: the SMS-SENT command does not take any arguments");
        return false;
    }

    std::vector<SentSmsRecord> smsList = app.getSentSmsHistory();
    if (smsList.empty())
    {
        std::cout << "No sent messages" << '\n';
        return true;
    }

    std::cout << "\n----- SENT MESSAGES -----" << '\n';
    std::cout << std::left << std::setw(18) << "Global ID" << " | "
              << std::setw(MAX_LENGTH_OF_IMSI_IMEI_MSISDN) << "To" << " | "
              << std::setw(15) << "Status" << " | "
              << "Text" << '\n';

    std::string idsString;
    for (const auto& sms : smsList)
    {
        idsString = covertIdsToString(sms.globalIds);
        if (idsString.length() > 15)
        {
            idsString = idsString.substr(0, 12) + "...";
        }

        //std::cout << std::left << std::setw(10);
        // if (sms.globalId.has_value())
        // {
        //     std::cout << sms.globalId.value();
        // }
        // if (!sms.globalIds)
        // {
        //     std::cout << "-";
        // }
        // for (size_t i = 0; i < sms.globalIds.value().size(); ++i)
        // {
        //     std::cout << sms.globalIds.value()[i];
        //     if (i < sms.globalIds.value().size() - 1)
        //     {
        //         std::cout << ", ";
        //     }
        // }
        
        std::cout << std::left << std::setw(18) << idsString  << " | "
                  << std::setw(MAX_LENGTH_OF_IMSI_IMEI_MSISDN) << sms.receiverMsisdn << " | "
                  << std::setw(15) << convertSmsDeliveryStatusToString(sms.status) << " | "
                  << sms.text << '\n';
    }
    return true;
}