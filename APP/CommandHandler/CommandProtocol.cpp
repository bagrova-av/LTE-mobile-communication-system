#include "CommandProtocol.h"
#include "../../COMMON/logging/Logger.h"

bool CommandProtocol::processValue(const std::string& value)
{
    Protocol newProtocol;
    if (value == "binary")
    {
        newProtocol = Protocol::BINARY;
    }
    else if (value == "json")
    {
        newProtocol = Protocol::JSON;
    }
    else
    {
        return false;
    }
    
    if (newProtocol == app.getProtocol())
    {
        Logger::info("Protocol is already " + value + ". No changes made");
    }
    else
    {
        app.setProtocol(newProtocol);
        Logger::info("Protocol successfully changed to " + value);
    }
    return true;
}