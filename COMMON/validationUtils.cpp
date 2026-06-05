#include "validationUtils.h"
#include "constants.h"
#include "./logging/Logger.h"
#include "../APP/app/StartupParams.h"

#include <format>

bool checkCorrectNameFile(const std::string& nameFile, const std::string& expectedExtension)
{
    std::string extension = expectedExtension;
    if (!extension.empty() && extension[0] != '.')
    {
        extension = '.' + extension;
    }

    size_t lenNameFile = nameFile.length();
    size_t lenExtention = extension.length();
    if (lenNameFile <= lenExtention || nameFile.substr(lenNameFile - lenExtention) != extension)
    {
        Logger::error("Validation error: you entered an incorrect filename with CLI.\n"
                        "The file must have an extension " + extension);
        return false;
    }
    return true;
}

bool validatePort(int32_t port)
{
    if (port < MIN_ALLOWED_PORT_NUMBER || port > MAX_ALLOWED_PORT_NUMBER)
    {
        Logger::error(std::format("Validation error: the port value cannot be {}. "
                                    "The port value must be an integer value between 1024 and 65535 " 
                                    "(in the range of registered and dynamic ports according to IANA)", 
                                    port));
        return false;
    }
    return true;
}

bool validateIMEI(const std::string& imei)
{
    if (imei.length() != MAX_LENGTH_OF_IMSI_AND_IMEI)
    {
        Logger::error(std::format("Validation error: the entered IMEI has a length of {}. "
                                    "It must contain exactly 15 characters", 
                                    imei.length()));
        return false;
    }
    
    for (char c : imei)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            Logger::error("Validation error: IMEI must contain only digits");
            return false;
        }
    }
    return true;
}

bool validateIMSI(const std::string& imsi)
{
    if (imsi.length() > MAX_LENGTH_OF_IMSI_AND_IMEI)
    {
        Logger::error(std::format("Validation error: the entered IMSI has a length of {}. "
                                    "It must contain no more than 15 characters",
                                    imsi.length()));
        return false;
    }
    else if (imsi.length() < MIN_LENGTH_OF_IMSI)
    {
        Logger::error(std::format("Validation error: the entered IMSI has a length of {}. "
                                    "It must contain at least 6 characters", 
                                    imsi.length()));
        return false;
    }
    for (char c : imsi)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            Logger::error("Validation error: IMSI must contain only digits");
            return false;
        }
    }
    return true;
}

bool validateServerConfig(const StartupParams& params)
{
    if (!params.port.has_value())
    {
        Logger::error("To initialize the server, the port must be set via the CLI flags when launching the application");
        return false;
    }
    if (!params.initialLocation.has_value())
    {
        Logger::error("To initialize the server, the position must be set via the CLI flags when launching the application");
        return false;
    }
    return true;
}