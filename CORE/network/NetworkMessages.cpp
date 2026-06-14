#include "NetworkMessages.h"

std::string convertSmsDeliveryStatusToString(SmsDeliveryStatus status)
{
    switch (status)
    {
        case SmsDeliveryStatus::SENDING :
        {
            return "SENDING";
        }
        case SmsDeliveryStatus::SENT_TO_SERVER :
        {
            return "SENT_TO_SERVER";
        }
        case SmsDeliveryStatus::DELIVERED :
        {
            return "DELIVERED";
        }
        case SmsDeliveryStatus::FAILED :
        {
            return "FAILED";
        }
        default:
        {
            return "UNKNOWN";
        }
    }
}