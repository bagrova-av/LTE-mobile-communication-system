#include <sstream>
#include <cctype>
#include <unistd.h>
#include <tuple>

#include "./app/App.h"
#include "./CommandHandler/CommandHandler.h"
#include "../CORE/NetworkAddress.h"
#include "../../COMMON/utils.h"
#include "../COMMON/parserUtils.h"
#include "../COMMON/ConsoleProvider.h"

struct AppLoadedConfig
{
    AppConfig appConfig;
    LocationPoint3D initialLocation;
    std::map<size_t, AddressBookEntry> addressBook;
};

std::optional<AppLoadedConfig> loadConfiguration(int argc, char** argv)
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
        if (!parseAppJSON(configPath, params.value()))
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

    std::map<size_t, AddressBookEntry> addressBook;
    if (!params.value().addressBookPath.empty())
    {
        if (!parseAddressBook(params.value().addressBookPath, appConfig.value().imsi, addressBook))
        {
            return std::nullopt;
        }
    }

    return AppLoadedConfig{ std::move(appConfig.value()),
                            std::move(params.value().initialLocation.value()),
                            std::move(addressBook) };
}

int main(int argc, char** argv)
{
    Logger::setMinimumLevel(LogLevel::DEBUG);
    initLogger(argv[0], "telecomAppClient_PID");

    ConsoleProvider::setInvitation("Please, enter the command: ");

    auto resultConfiguration = loadConfiguration(argc, argv);
    if (!resultConfiguration.has_value())
    {
        return 1;
    }

    AppState initialState{ resultConfiguration.value().initialLocation,
                           false,
                           Protocol::JSON,
                           std::nullopt };

    App app( std::move(resultConfiguration.value().appConfig),
             std::move(initialState),
             std::move(resultConfiguration.value().addressBook) );
    CommandHandler menu(app);
    menu.process();

    return 0;
}