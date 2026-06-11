#include "parserUtils.h"
#include "utils.h"
#include "validationUtils.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

std::optional<LocationPoint3D> parseLocation(const std::vector<CoordinatesType>& pointComponents)
{
    return parseLocationImpl<LocationPoint3D>(pointComponents);
}

bool handleStringArgument(int argc, char** argv, size_t& i,
                          const std::string& flagName, std::optional<std::string>& targetField)
{
    if (i + 1 >= argc)
    {
        Logger::error("Error: Missing value for " + flagName);
        return false;
    }

    targetField = argv[i+1];
    ++i;
    return true;
}

bool handleStringArgument(int argc, char** argv, size_t& i,
                          const std::string& flagName, std::string& targetField)
{
    if (i + 1 >= argc)
    {
        Logger::error("Error: Missing value for " + flagName);
        return false;
    }

    targetField = argv[i+1];
    ++i;
    return true;
}

bool handleIMEIArgument(int argc, char** argv, size_t& i,
                          const std::string& flagName, std::optional<std::string>& targetField)
{
    if (i + 1 >= argc)
    {
        Logger::error("Error: Missing value for " + flagName);
        return false;
    }

    targetField = argv[i+1];
    if (!validateIMEI(targetField.value()))
    {
        return false;
    }
    ++i;
    return true;
}

bool handleIMSIArgument(int argc, char** argv, size_t& i,
                          const std::string& flagName, std::optional<std::string>& targetField)
{
    if (i + 1 >= argc)
    {
        Logger::error("Error: Missing value for " + flagName);
        return false;
    }

    targetField = argv[i+1];
    if (!validateIMSI(targetField.value()))
    {
        return false;
    }
    ++i;
    return true;
}

bool handleMSISDNArgument(int argc, char** argv, size_t& i,
                          const std::string& flagName, std::optional<std::string>& targetField)
{
    if (i + 1 >= argc)
    {
        Logger::error("Error: Missing value for " + flagName);
        return false;
    }

    targetField = argv[i+1];
    if (!validateMSISDN(targetField.value()))
    {
        return false;
    }
    ++i;
    return true;
}

bool handleLocationArgument(int argc, char** argv, size_t& i,
                          const std::string& flagName, std::optional<LocationPoint3D>& targetField)
{
    if (i + 3 >= argc)
    {
        Logger::error("Error: Missing value for " + flagName);
        return false;
    }

    auto x = convertFromString<CoordinatesType>(argv[i+1]);
    if (!x.has_value())
    {
        Logger::error("Validation error: the location coordinate X contains non-numeric characters");
        return false;
    }
    auto y = convertFromString<CoordinatesType>(argv[i+2]);
    if (!y.has_value())
    {
        Logger::error("Validation error: the location coordinate Y contains non-numeric characters");
        return false;
    }
    auto z = convertFromString<CoordinatesType>(argv[i+3]);
    if (!z.has_value())
    {
        Logger::error("Validation error: the location coordinate Z contains non-numeric characters");
        return false;
    }

    targetField = LocationPoint3D{x.value(), y.value(), z.value()};
    i += 3;
    return true;
}

bool handleConfigPathArgument(int argc, char** argv, size_t& i, 
                          const std::string& flagName, std::string& targetField)
{
    if (i + 1 >= argc)
    {
        Logger::error("Error: Missing value for " + flagName);
        return false;
    }

    targetField = argv[i+1];
    if (!checkCorrectNameFile(targetField, ".json"))
    {
        return false;
    }
    ++i;
    return true;
}

std::optional<StartupParams> parseCLI(int argc, char** argv, std::string& configPath)
{
    StartupParams appConfig;
    std::string arg = "";

    for (size_t i = 1; i < argc; ++i)
    {
        arg = argv[i];

        if (arg == "-a" || arg == "--ip")
        {
            if (!handleStringArgument(argc, argv, i, arg, appConfig.ip))
            {
                return std::nullopt;
            }
        }
        else if (arg == "-p" || arg == "--port")
        {
            if (!handleStringArgument(argc, argv, i, arg, appConfig.port))
            {
                return std::nullopt;
            }
        }
        else if (arg == "-e" || arg == "--imei")
        {
            if (!handleIMEIArgument(argc, argv, i, arg, appConfig.imei))
            {
                return std::nullopt;
            }
        }
        else if (arg == "-i" || arg == "--imsi")
        {
            if (!handleIMSIArgument(argc, argv, i, arg, appConfig.imsi))
            {
                return std::nullopt;
            }
        }
        else if (arg == "-l" || arg == "--loc")
        {
            if (!handleLocationArgument(argc, argv, i, arg, appConfig.initialLocation))
            {
                return std::nullopt;
            }
        }
        else if (arg == "-k" || arg == "--config")
        {
            if (!handleConfigPathArgument(argc, argv, i, arg, configPath))
            {
                return std::nullopt;
            }
        }
        else if (arg == "-m" || arg == "--msisdn")
        {
            if (!handleMSISDNArgument(argc, argv, i, arg, appConfig.msisdn))
            {
                return std::nullopt;
            }
        }
        else if (arg == "-n" || arg == "--nodes")
        {
            if (!handleStringArgument(argc, argv, i, arg, appConfig.nodes))
            {
                return std::nullopt;
            }
        }
        else 
        {
            Logger::error("Error: unknown argument (" + arg + "). "
                          "When launching the application, the following flags are allowed: "
                          "-a/--ip, -p/--port, -e/--imei, -i/--imsi, -m/--msisdn, -l/--loc, -k/--config, -n/--nodes");
            return std::nullopt;
        }
    }
    return appConfig;
}

bool parseJSON(const std::string& configPath, StartupParams& appConfig)
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open())
    {
        Logger::error("Error: could not open config file: " + configPath);
    }

    json data;
    try
    {
        configFile >> data;
        if (!appConfig.ip.has_value() && data.contains("ip"))
        {
            appConfig.ip = data["ip"].get<std::string>();
        }
        if (!appConfig.port.has_value() && data.contains("port"))
        {
            if (!data["port"].is_number_integer())
            {
                Logger::error("Validation error: port must be an integer value");
                return false;
            }

            appConfig.port = std::to_string(data["port"].get<int>());
        }
        if (!appConfig.imei.has_value() && data.contains("imei"))
        {
            std::string imeiTemp = data["imei"].get<std::string>();
            if (!validateIMEI(imeiTemp))
            {
                return false;
            }
            appConfig.imei = imeiTemp;
        }
        if (!appConfig.imsi.has_value() && data.contains("imsi"))
        {
            std::string imsiTemp = data["imsi"].get<std::string>();
            if (!validateIMSI(imsiTemp))
            {
                return false;
            }
            appConfig.imsi = imsiTemp;
        }
        if (!appConfig.msisdn.has_value() && data.contains("msisdn"))
        {
            std::string msisdnTemp = data["msisdn"].get<std::string>();
            if (!validateMSISDN(msisdnTemp))
            {
                return false;
            }
            appConfig.msisdn = msisdnTemp;
        }
        if (!appConfig.initialLocation.has_value() && data.contains("loc"))
        {
            appConfig.initialLocation = parseLocation(data["loc"].get<std::vector<CoordinatesType>>());
            if (!appConfig.initialLocation.has_value())
            {
                return false;
            }
        }
    }
    catch (const json::exception& ex)
    {
        Logger::error(std::format("Error: Failed to process JSON file.\nDetails: {}", 
                                    ex.what()));
        return false;
    }

    configFile.close();
    return true;
}

std::optional<AppConfig> buildFinalConfig(const StartupParams& params)
{
    if (!params.ip.has_value() || !params.port.has_value() || !params.imei.has_value()
        || !params.imsi.has_value() || !params.msisdn.has_value() || !params.initialLocation.has_value())
    {
        Logger::error("Error: missing required parameters. Please, provide them via CLI or JSON config");
        return std::nullopt;
    }

    try
    {
        return AppConfig{
            NetworkAddress(params.ip.value(), params.port.value()),
            params.imsi.value(),
            params.imei.value(),
            params.msisdn.value(),
            params.nodes
        };
    }
    catch(const std::exception& ex)
    {
        std::string errorMessage = ex.what();
        if (!errorMessage.empty()) 
        {
            Logger::error(errorMessage);
        }
        return std::nullopt;
    }
}

std::vector<std::string> splitCommand(const std::string& commandStr)
{
    std::vector<std::string> commandParts;
    size_t pos = 0;
    size_t commandLength = commandStr.length();

    while (pos < commandLength)
    {
        while (pos < commandLength && std::isspace(commandStr[pos]))
        {
            ++pos;
        }
        if (pos >= commandLength)
        {
            break;
        }

        size_t start = pos;
        while (pos < commandLength && !std::isspace(commandStr[pos]))
        {
            ++pos;
        }
        commandParts.emplace_back(commandStr.substr(start, pos - start));
    }
    return commandParts;
}

int32_t parsePort(const std::string& portStr)
{
    auto port = convertFromString<int>(portStr);
    if (!port.has_value())
    {
        Logger::error("Validation error: the port value must be an integer value");
        throw std::invalid_argument("");
    }
    return port.value();
}

std::optional<std::vector<BaseStationConfig>> parseBaseStationsJSON(const std::string& configPath)
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open())
    {
        Logger::error("Error: could not open baseStations config file: " + configPath);
        return std::nullopt;
    }

    json data;
    std::vector<BaseStationConfig> baseStations;
    try
    {
        configFile >> data;
        if (!data.contains("baseStations") || !data["baseStations"].is_array())
        {
            return std::nullopt;
        }

        const auto& items = data["baseStations"];
        baseStations.reserve(items.size());
        for (const auto& item : items)
        {
            BaseStationId id = item["id"].get<BaseStationId>();
            MmeId mmeId = item["mmeId"].get<MmeId>();
            std::vector<CoordinatesType> positionComponents = item["position"].get<std::vector<CoordinatesType>>();
            CoordinatesType coverageRadius = item["coverageRadius"].get<CoordinatesType>();
            size_t maxSupportedConnections = item["maxSupportedConnections"].get<size_t>();
            size_t bufferBytesSizePerConnection = item["bufferBytesSizePerConnection"].get<size_t>();
            LocationPoint3D position;
            position.updateFromVector(positionComponents);
            baseStations.emplace_back(BaseStationConfig{id,
                                                        mmeId,
                                                        position,
                                                        coverageRadius,
                                                        maxSupportedConnections,
                                                        bufferBytesSizePerConnection}
            );
        }
    }
    catch(const std::exception& ex)
    {
        Logger::error(std::format("Error: Failed to process JSON file with baseStations config.\nDetails: {}", 
                                    ex.what()));
        return std::nullopt;
    }

    configFile.close();
    return baseStations;
}