#include <iostream>
#include <string>

#include <optional>
#include <memory>

#include <filesystem>
#include <unistd.h>

#include "ServerApp.h"
#include "../APP/app/StartupParams.h"
#include "../COMMON/parserUtils.h"
#include "../COMMON/validationUtils.h"
#include "../COMMON/logging/FileLogger.h"
#include "../COMMON/logging/Logger.h"

int main(int argc, char** argv)
{
    Logger::setMinimumLevel(LogLevel::DEBUG);

    std::filesystem::path execPath = argv[0];
    std::string logFileName = "TelecomAppServer_PID" + std::to_string(getpid()) + ".log";
    std::filesystem::path logPath = execPath.parent_path() / logFileName;

    auto fileLogger = std::make_unique<FileLogger>(logPath.string());
    Logger::setLogger(std::move(fileLogger));

    std::cout << "==========================================" << '\n';
    std::cout << "  Log file: " << logFileName << '\n';
    std::cout << "  Process ID (PID): " << getpid() << '\n';
    std::cout << "==========================================" << '\n';

    Logger::info("ServerApp started");

    Logger::info("Loading parameters from CLI for server");
    std::string configPath = "";

    std::optional<StartupParams> params = parseCLI(argc, argv, configPath);
    if (!params.has_value())
    {
        Logger::error("Failed to read CLI parameters. Shutting down");
        return 1;
    }
    if (!validateServerConfig(params.value()))
    {
        return 1;
    }
    Logger::info("All entered parameters collected successfully");

    int32_t serverPort = -1;
    Logger::info("Validating parameters");
    try
    {
        serverPort = parsePort(params.value().port.value());
    }
    catch(const std::exception& ex)
    {
        return 1;
    }

    if (!validatePort(serverPort))
    {
        return 1;
    }

    ServerApp server(static_cast<uint16_t>(serverPort),
                     params.value().initialLocation.value());
    server.process();

    Logger::info("ServerApp shutting down\n");
    return 0;
}