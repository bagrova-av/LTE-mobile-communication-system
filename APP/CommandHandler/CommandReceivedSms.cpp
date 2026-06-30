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

    std::cout << "\n----- RECEIVED MESSAGES -----" << '\n';
    size_t totalReceivedSms = app.forEachReceivedSms(
                                [this](const ReceivedSmsRecord& sms)
                                {
                                    std::cout << "From: " << sms.senderMsisdn
                                            << " | Text: " << sms.text << '\n';
                                }
    );
    if (totalReceivedSms == 0)
    {
        std::cout << "No received messages" << '\n';
    }

    return true;
}