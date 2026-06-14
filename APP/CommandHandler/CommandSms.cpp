#include "CommandSms.h"
#include "../../COMMON/logging/Logger.h"
#include "../../COMMON/validationUtils.h"
#include "../../COMMON/utils.h"

#include <iostream>

bool CommandSms::execute(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        Logger::warning("Error: the SMS command takes 2 arguments "
                        "(the receiver's MSISDN and a text message to him)");
        return false;
    }

    MSISDN receiverMsisdn = args[0];
    if (!validateMSISDN(receiverMsisdn))
    {
        return false;
    }

    size_t countComponents = args.size();
    std::string text = "";
    if (countComponents > 1)
    {
        for (size_t i = 1; i < countComponents; ++i)
        {
            text += args[i];
            if (i != countComponents - 1)
            {
                text += " ";
            }
        }
    }

    if (isBlankString(text))
    {
        std::cout << "Enter SMS text: ";
        if (!std::getline(std::cin, text) || isBlankString(text))
        {
            Logger::warning("Error: the SMS command takes 2 arguments "
                        "(the receiver's MSISDN and a text message to him) "
                        "and SMS text cannot be empty");
            return false; 
        }
    }

    SmsId tempId = app.generateNextLocalSmsId();
    app.addSentSmsRecord(SentSmsRecord{tempId, std::nullopt,
                                       receiverMsisdn, text,
                                       SmsDeliveryStatus::SENDING});
    app.sendSmsTaskToServerAsync(SmsTaskData{receiverMsisdn, tempId, text});

    return true;
}