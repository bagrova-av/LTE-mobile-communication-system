#include "CommandSentSms.h"
#include "../../COMMON/constants.h"
#include "../../CORE/network/NetworkMessages.h"
#include "../../COMMON/logging/Logger.h"

#include <iostream>
#include <iomanip>

std::string CommandSentSms::covertIdsToString(const std::vector<SmsId>& ids)
{
    if (ids.empty())
    {
        return std::string("-");
    }

    std::string res;
    size_t countIds = ids.size();
    for (size_t i = 0; i < countIds; ++i)
    {
        res += std::to_string(ids[i]);
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

    std::cout << "\n----- SENT MESSAGES -----" << '\n';
    std::cout << std::left << std::setw(18) << "Global ID" << " | "
              << std::setw(MAX_LENGTH_OF_IMSI_IMEI_MSISDN) << "To" << " | "
              << std::setw(15) << "Status" << " | "
              << "Text" << '\n';

    size_t totalSentSms = app.forEachSentSms(
                            [this](const SentSmsRecord& sms)
                            {
                                std::string idsString = "-";
                                if (sms.globalIds.has_value())
                                {
                                    idsString = covertIdsToString(sms.globalIds.value());
                                }

                                if (idsString.length() > 15)
                                {
                                    idsString = idsString.substr(0, 12) + "...";
                                }

                                std::cout << std::left << std::setw(18) << idsString  << " | "
                                        << std::setw(MAX_LENGTH_OF_IMSI_IMEI_MSISDN) << sms.receiverMsisdn << " | "
                                        << std::setw(15) << convertSmsDeliveryStatusToString(sms.status) << " | "
                                        << sms.text << '\n';
                            }
    );
    if (totalSentSms == 0)
    {
        std::cout << "No sent messages" << '\n';
    }

    return true;
}