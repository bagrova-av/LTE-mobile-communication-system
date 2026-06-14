#include "CommandSentSms.h"
#include "../../COMMON/constants.h"
#include "../../CORE/network/NetworkMessages.h"

#include <iostream>
#include <iomanip>

bool CommandSentSms::execute(const std::vector<std::string>&)
{
    std::vector<SentSmsRecord> smsList = app.getSentSmsHistory();
    if (smsList.empty())
    {
        std::cout << "No sent messages" << '\n';
        return true;
    }

    std::cout << "\n--- SENT MESSAGES ---" << '\n';
    std::cout << std::left << std::setw(10) << "ID" << " | "
              << std::setw(MAX_LENGTH_OF_IMSI_IMEI_MSISDN) << "To" << " | "
              << std::setw(15) << "Status" << " | "
              << "Text" << '\n';

    for (const auto& sms : smsList)
    {
        std::cout << std::left << std::setw(10);
        if (sms.globalId.has_value())
        {
            std::cout << sms.globalId.value();
        }
        else
        {
            std::cout << "-";
        }
        std::cout << " | "
                  << std::setw(MAX_LENGTH_OF_IMSI_IMEI_MSISDN) << sms.receiverMsisdn << " | "
                  << std::setw(15) << convertSmsDeliveryStatusToString(sms.status) << " | "
                  << sms.text << '\n';
    }
    return true;
}