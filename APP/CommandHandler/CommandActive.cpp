#include "CommandActive.h"

bool CommandActive::processValue(const std::string& value)
{
    bool newState;
    std::string stateStr = "";
    if (value == "true" || value == "1")
    {
        newState = true;
        stateStr = "active";
    }
    else if (value == "false" || value == "0")
    {
        newState = false;
        stateStr = "inactive";
    }
    else
    {
        return false;
    }
    
    if (newState == app.getStatus())
    {
        Logger::info("Application status is already " + stateStr + ". No changes made");
    } 
    else
    {
        app.setStatus(newState);
        Logger::info("Application status successfully changed to " + stateStr);
    }
    return true;
}
