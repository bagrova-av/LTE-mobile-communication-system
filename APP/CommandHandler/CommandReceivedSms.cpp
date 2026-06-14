#include "CommandReceivedSms.h"

#include <iostream>

bool CommandReceivedSms::execute(const std::vector<std::string>& args)
{
    std::vector<ReceivedSmsRecord> smsList = app.getReceivedSmsHistory();
    if (smsList.empty())
    {
        std::cout << "No received messages" << '\n';
        return true;
    }

    std::cout << "\n--- INBOX ---" << '\n';
    for (const auto& sms : smsList)
    {
        std::cout << "From: " << sms.senderMsisdn
                  << " | Text: " << sms.text << '\n';
    }
    return true;
}