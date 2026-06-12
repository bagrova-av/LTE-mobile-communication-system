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
                                    "The port value must be an integer value between {} and {} " 
                                    "(in the range of registered and dynamic ports according to IANA)", 
                                    port, MIN_ALLOWED_PORT_NUMBER, MAX_ALLOWED_PORT_NUMBER));
        return false;
    }
    return true;
}

bool validateIMEI(const std::string& imei)
{
    if (imei.length() != MAX_LENGTH_OF_IMSI_IMEI_MSISDN)
    {
        Logger::error(std::format("Validation error: the entered IMEI has a length of {}. "
                                    "It must contain exactly {} characters", 
                                    imei.length(), MAX_LENGTH_OF_IMSI_IMEI_MSISDN));
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
    if (imsi.length() > MAX_LENGTH_OF_IMSI_IMEI_MSISDN)
    {
        Logger::error(std::format("Validation error: the entered IMSI has a length of {}. "
                                    "It must contain no more than {} characters",
                                    imsi.length(), MAX_LENGTH_OF_IMSI_IMEI_MSISDN));
        return false;
    }
    else if (imsi.length() < MIN_LENGTH_OF_IMSI)
    {
        Logger::error(std::format("Validation error: the entered IMSI has a length of {}. "
                                    "It must contain at least {} characters", 
                                    imsi.length(), MIN_LENGTH_OF_IMSI));
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

bool validateMSISDN(const std::string& msisdn)
{
    if (msisdn.length() > MAX_LENGTH_OF_IMSI_IMEI_MSISDN)
    {
        Logger::error(std::format("Validation error: the entered MSISDN has a length of {}. "
                                    "It must contain no more than {} characters",
                                    msisdn.length(), MAX_LENGTH_OF_IMSI_IMEI_MSISDN));
        return false;
    }
    else if (msisdn.length() < MIN_LENGTH_OF_MSISDN)
    {
        Logger::error(std::format("Validation error: the entered MSISDN has a length of {}. "
                                    "It must contain at least {} characters",
                                    msisdn.length(), MIN_LENGTH_OF_MSISDN));
        return false;
    }
    for (char c : msisdn)
    {
        if (!std::isdigit(static_cast<unsigned char>(c)))
        {
            Logger::error("Validation error: MSISDN must contain only digits");
            return false;
        }
    }
    return true;
}

bool validateServerConfig(const StartupParams& params, const std::string& configFile)
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
    if (configFile.empty())
    {
        Logger::error("To initialize the server, evolved packet core config (EPC) file must be set "
                      "via the CLI flags when launching the application");
        return false;
    }
    return true;
}