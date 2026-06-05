#include "SingleArgCommand.h"

bool SingleArgCommand::execute(const std::vector<std::string>& args)
{
    if (args.size() != 1)
    {
        Logger::warning("Error: the " + commandName + " command takes 1 argument "
                        "(which can be " + allowedValuesMessage + ")");
        return false;
    }

    std::string value = toLowerCase(args[0]);

    if (!processValue(value))
    {
        Logger::warning("Error: the " + commandName + " argument must be " + allowedValuesMessage);
        return false;
    }
    return true;
}