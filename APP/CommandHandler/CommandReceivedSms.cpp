#include "CommandReceivedSms.h"
#include "../../COMMON/logging/Logger.h"

#include <iostream>

bool CommandReceivedSms::execute(const std::vector<std::string>& args)
{
    if (!args.empty())
    {
        Logger::warning("Error: the SMS-RECEIVED command does not take any arguments");
        return false;
    }

    std::vector<ReceivedSmsRecord> smsList = app.getReceivedSmsHistory();
    if (smsList.empty())
    {
        std::cout << "No received messages" << '\n';
        return true;
    }

    std::cout << "\n----- RECEIVED MESSAGES -----" << '\n';
    for (const auto& sms : smsList)
    {
        std::cout << "From: " << sms.senderMsisdn
                  << " | Text: " << sms.text << '\n';
    }
    return true;
}