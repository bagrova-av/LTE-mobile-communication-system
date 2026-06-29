#include "CommandExit.h"
#include "../../COMMON/logging/Logger.h"

bool CommandExit::execute(const std::vector<std::string>& args)
{
    if (!args.empty())
    {
        Logger::warning("Error: the EXIT command does not take any arguments");
        return false;
    }
    return true;
}