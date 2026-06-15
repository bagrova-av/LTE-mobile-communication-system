#include <iostream>
#include <string>

#include <optional>
#include <memory>
#include <tuple>

#include <filesystem>
#include <unistd.h>
#include <csignal>

#include "ServerApp.h"
#include "../APP/app/StartupParams.h"
#include "../COMMON/parserUtils.h"
#include "../COMMON/validationUtils.h"
#include "../COMMON/logging/FileLogger.h"
#include "../COMMON/logging/Logger.h"
#include "../COMMON/utils.h"

static ServerApp* globalServerAppPtr = nullptr;

void signalHandler(int signum)
{
    if (globalServerAppPtr)
    {
        Logger::info("Signal (" + std::to_string(signum) + ") received. Stopping server...");
        globalServerAppPtr->stop();
    }
}

std::optional<std::tuple<uint32_t, LocationPoint3D, EpcConfig>> loadConfiguration(int argc, char** argv)
{
    Logger::info("Loading parameters for server");
    std::string configPath = "";
    std::optional<StartupParams> params = parseCLI(argc, argv, configPath);
    if (!params.has_value())
    {
        Logger::error("Failed to read CLI parameters. Shutting down");
        return std::nullopt;
    }

    if (!validateServerConfig(params.value(), configPath))
    {
        return std::nullopt;
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
        return std::nullopt;
    }

    if (!validatePort(serverPort))
    {
        return std::nullopt;
    }

    EpcConfig epcConfig;
    if (!parseEpcJSON(configPath, epcConfig))
    {
        return std::nullopt;
    }

    return std::make_tuple(serverPort, params.value().initialLocation.value(), epcConfig);
}

int main(int argc, char** argv)
{
    Logger::setMinimumLevel(LogLevel::DEBUG);
    initLogger(argv[0], "TelecomAppServer_PID");

    auto resultConfiguration = loadConfiguration(argc, argv);
    if (!resultConfiguration.has_value())
    {
        return 1;
    }
    const auto& [serverPort, initialLocation, epcConfig] = resultConfiguration.value();

    try
    {
        ServerApp server(static_cast<uint16_t>(serverPort), initialLocation, epcConfig);
        globalServerAppPtr = &server;

        std::signal(SIGINT, signalHandler);

        server.process();
    }
    catch(const std::exception& ex)
    {
        Logger::error(ex.what());
    }
    Logger::info("ServerApp shutting down\n");
    return 0;
}