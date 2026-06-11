#include <iostream>
#include <sstream>
#include <cctype>

#include <filesystem>
#include <unistd.h>

#include "./app/App.h"
#include "./CommandHandler/CommandHandler.h"
#include "../CORE/NetworkAddress.h"

#include "../COMMON/logging/FileLogger.h"

#include "../COMMON/parserUtils.h"

void initLogger(const std::string& executablePath)
{
    std::filesystem::path execPath = executablePath;
    std::string logFileName = "telecomAppClient_PID" + std::to_string(getpid()) + ".log";
    std::filesystem::path logPath = execPath.parent_path() / logFileName;

    auto fileLogger = std::make_unique<FileLogger>(logPath.string());
    Logger::setLogger(std::move(fileLogger));

    std::cout << "==========================================" << '\n';
    std::cout << "  Log file: " << logFileName << '\n';
    std::cout << "  Process ID (PID): " << getpid() << '\n';
    std::cout << "==========================================" << '\n';
}

std::optional<std::pair<AppConfig, LocationPoint3D>> loadConfiguration(int argc, char** argv)
{
    Logger::info("Loading parameters from CLI");
    std::string configPath = "";
    std::optional<StartupParams> params = parseCLI(argc, argv, configPath);
    if (!params.has_value())
    {
        Logger::error("Failed to read CLI parameters. Shutting down");
        return std::nullopt;
    }

    if (!configPath.empty())
    {
        Logger::info("Loading parameters from " + configPath);
        if (!parseJSON(configPath, params.value()))
        {
            Logger::error("Failed to read parameters from " + configPath + ". Shutting down");
            return std::nullopt;
        }
    }
    Logger::info("All entered parameters collected successfully");
    
    Logger::info("Validating parameters and building network configuration");
    auto appConfig = buildFinalConfig(params.value());
    if (!appConfig.has_value())
    {
        return std::nullopt;
    }

    return std::make_pair(appConfig.value(), params.value().initialLocation.value());
}

int main(int argc, char** argv)
{
    Logger::setMinimumLevel(LogLevel::DEBUG);
    initLogger(argv[0]);

    auto resultConfiguration = loadConfiguration(argc, argv);
    if (!resultConfiguration.has_value())
    {
        return 1;
    }
    const auto& [appConfig, initialLocation] = resultConfiguration.value();

    AppState initialState{ initialLocation, false, Protocol::JSON, std::nullopt };

    App app(appConfig, initialState);
    CommandHandler menu(app);
    menu.process();

    return 0;
}