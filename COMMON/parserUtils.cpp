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

bool parseAppJSON(const std::string& configPath, StartupParams& appConfig)
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

bool parseEpcJSON(const std::string& configPath, EpcConfig& epcConfig)
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open())
    {
        Logger::error("Error: could not open evolved packet core config file: " + configPath);
        return false;
    }

    json data;
    try
    {
        configFile >> data;

        if (!data.contains("simtel_ttl") || !data["simtel_ttl"].is_number_unsigned())
        {
            return false;
        }
        epcConfig.simtelTtl = data["simtel_ttl"].get<Seconds>();
        
        if (!data.contains("hlrEir"))
        {
            return false;
        }
        auto hlrEir = data["hlrEir"];

        if (!hlrEir.contains("format"))
        {
            return false;
        }
        epcConfig.hlrEir.format = hlrEir["format"].get<std::string>();

        if (!hlrEir.contains("provider"))
        {
            return false;
        }
        epcConfig.hlrEir.provider = hlrEir["provider"].get<std::string>();

        if (!hlrEir.contains("path"))
        {
            return false;
        }
        epcConfig.hlrEir.path = hlrEir["path"].get<std::string>();

        if (!data.contains("nodes_file"))
        {
            return false;
        }
        epcConfig.nodesConfigFile = data["nodes_file"].get<std::string>();

        if (!data.contains("mme_config"))
        {
            return false;
        }
        epcConfig.mmeConfigFile = data["mme_config"].get<std::string>();
    }
    catch(const std::exception& ex)
    {
        Logger::error(std::format("Error: Failed to process JSON file with  evolved packet core config.\nDetails: {}", 
                                    ex.what()));
        return false;
    }

    configFile.close();
    return true;
}

bool parseBaseStationsJSON(const std::string& configPath, std::vector<BaseStationConfig>& bsConfigs)
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open())
    {
        Logger::error("Error: could not open baseStations config file: " + configPath);
        return false;
    }

    json data;
    try
    {
        configFile >> data;
        if (!data.contains("baseStations") || !data["baseStations"].is_array())
        {
            return false;
        }

        const auto& items = data["baseStations"];
        bsConfigs.reserve(items.size());
        for (const auto& item : items)
        {
            if (!item.contains("id") || !item["id"].is_number_unsigned())
            {
                Logger::error("Validation error: base station ID must be an integer value");
                return false;
            }
            BaseStationId id = item["id"].get<BaseStationId>();

            if (!item.contains("mmeId") || !item["mmeId"].is_number_unsigned())
            {
                Logger::error("Validation error: mobility management entity ID must be an integer value");
                return false;
            }
            MmeId mmeId = item["mmeId"].get<MmeId>();

            if (!item.contains("position"))
            {
                return false;
            }
            std::optional<LocationPoint3D> position = parseLocation(item["position"].get<std::vector<CoordinatesType>>());
            if (!position.has_value())
            {
                return false;
            }

            if (!item.contains("coverageRadius") || !item["coverageRadius"].is_number()
                || item["coverageRadius"] < 0)
            {
                Logger::error("Validation error: coverage radius must be a positive number");
                return false;
            }
            CoordinatesType coverageRadius = item["coverageRadius"].get<CoordinatesType>();

            if (!item.contains("maxSupportedConnections")
                || !item["maxSupportedConnections"].is_number_unsigned())
            {
                Logger::error("Validation error: number with max supported connections must be an integer value");
                return false;
            }
            size_t maxSupportedConnections = item["maxSupportedConnections"].get<size_t>();

            if (!item.contains("bufferBytesSizePerConnection")
                || !item["bufferBytesSizePerConnection"].is_number_unsigned())
            {
                Logger::error("Validation error: number with buffer bytes size per connection must be an integer value");
                return false;
            }
            size_t bufferBytesSizePerConnection = item["bufferBytesSizePerConnection"].get<size_t>();

            bsConfigs.emplace_back(BaseStationConfig{id,
                                                    mmeId,
                                                    position.value(),
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
        return false;
    }

    configFile.close();
    return true;
}

// bool parseHlrEirRecordsJSON(const std::string& configPath, std::vector<HlrEirRecord>& records)
// {
//     std::ifstream configFile(configPath);
//     if (!configFile.is_open())
//     {
//         Logger::error("Error: could not open HLR(+EIR): " + configPath);
//         return false;
//     }

//     json data;
//     try
//     {
//         configFile >> data;
//         for (const auto& item : data)
//         {
//             if (!item.contains("imsi"))
//             {
//                 return false;
//             }
//             std::string imsiTemp = item["imsi"].get<std::string>();
//             if (!validateIMSI(imsiTemp))
//             {
//                 return false;
//             }

//             if (!item.contains("imei"))
//             {
//                 return false;
//             }
//             std::string imeiTemp = item["imei"].get<std::string>();
//             if (!validateIMEI(imeiTemp))
//             {
//                 return false;
//             }

//             if (!item.contains("msisdn"))
//             {
//                 return false;
//             }
//             std::string msisdnTemp = item["msisdn"].get<std::string>();
//             if (!validateMSISDN(msisdnTemp))
//             {
//                 return false;
//             }

//             if (!item.contains("status"))
//             {
//                 return false;
//             }
//             std::optional<SubscriberStatus> status = convertStringToStatus(item["status"].get<std::string>());
//             if (!status.has_value())
//             {
//                 return false;
//             }

//             IdentitySubscriberData identityData{imsiTemp,
//                                                 imeiTemp,
//                                                 msisdnTemp,
//                                                 status.value()};

//             SessionSubscriberData sessionData;
//             sessionData.lastActivityTime = std::nullopt;
//             sessionData.currentMmeId = std::nullopt;

//             records.emplace_back(HlrEirRecord{identityData, sessionData});
//         }
//     }
//     catch(const std::exception& ex)
//     {
//         Logger::error(std::format("Error: Failed to process JSON file with HLR(+EIR).\nDetails: {}", 
//                                     ex.what()));
//         return false;
//     }

//     configFile.close();
//     return true;
// }

bool parseMmeJSON(const std::string& configPath, std::vector<MmeConfig>& mmes)
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open())
    {
        Logger::error("Error: could not open MMEs config file: " + configPath);
        return false;
    }

    json data;
    try
    {
        configFile >> data;
        for (const auto& item : data)
        {
            if (!item.contains("id") || !item["id"].is_number_unsigned())
            {
                Logger::error("Validation error: MME ID must be an integer value");
                return false;
            }

            if (!item.contains("maximum_records_in_vlr")
                || !item["maximum_records_in_vlr"].is_number_unsigned())
            {
                Logger::error("Validation error: number with maximum records in VLR must be an integer value");
                return false;
            }

            mmes.emplace_back(item["id"].get<int>(), item["maximum_records_in_vlr"].get<int>());
        }
    }
    catch(const std::exception& ex)
    {
        Logger::error(std::format("Error: Failed to process JSON file with MMEs configs.\nDetails: {}",
                                    ex.what()));
        return false;
    }

    configFile.close();
    return true;
}