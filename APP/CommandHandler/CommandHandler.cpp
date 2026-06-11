#include "CommandHandler.h"
#include "../../COMMON/utils.h"
#include "../../COMMON/parserUtils.h"
#include "../../COMMON/logging/Logger.h"
#include "CommandActive.h"
#include "CommandMove.h"
#include "CommandProtocol.h"
#include "CommandExit.h"

#include <iostream>
#include <sstream>

CommandHandler::CommandHandler(App& app) :
    app(app)
{
    commands.emplace("active", std::make_unique<CommandActive>(app) );
    commands.emplace("move", std::make_unique<CommandMove>(app) );
    commands.emplace("protocol", std::make_unique<CommandProtocol>(app) );
    commands.emplace("exit", std::make_unique<CommandExit>() );
}

void CommandHandler::printMenuHeader() const
{
    std::cout << "\n===== Status: ";
    if (app.getStatus())
    {
        std::cout << "active";
    }
    else
    {
        std::cout << "inactive";
    }
    std::cout << " | "
                << "IMSI: " << app.getIMSI() << " | "
                << "Location: " << app.getLocation() << " | "
                << "Protocol: " << app.getProtocol()
                << " =====\n";
    std::cout << "Available Commands (the commands are case-insensitive):\n"
                << "EXIT - exit the application\n" 
                << "ACTIVE <state> - change the application state (<state> can be: true/false/0/1)\n" 
                << "MOVE <newX> [newY] [newZ] - change the value of the position vector (1–3 values)\n" 
                << "   MOVE <newX> - change only the 'x' coordinate\n"
                << "   MOVE <newX> <newY> - change the 'x', 'y' coordinates\n"
                << "   MOVE <newX> <newY> <newZ> - change the 'x', 'y', 'z' coordinates\n"
                << "PROTOCOL <type> - set data transfer format (<type> can be: binary/json)\n\n";
}

void CommandHandler::process()
{
    std::cout << "App started successfully with params:\n"
              << " IP Address : " << app.getIP() << '\n'
              << " Port       : " << app.getPort() << '\n'
              << " IMEI       : " << app.getIMEI() << '\n'
              << " MSISDN     : " << app.getMSISDN() << '\n'
              << " Nodes      : " << app.getNodes();

    std::string commandStr;
    bool isSuccessful = true;

    while (true)
    {
        app.processResponses();

        printMenuHeader();
        std::cout << "Please, enter the command: ";

        if (!std::getline(std::cin, commandStr))
        {
            std::cout << '\n';
            break;
        }

        std::vector<std::string> commandParts = splitCommand(commandStr);
        if (commandParts.empty())
        {
            continue;
        }
        std::string commandName = toLowerCase(commandParts[0]);
        commandParts.erase(commandParts.begin());

        auto it = commands.find(commandName);

        if (it != commands.cend())
        {
            Logger::info("Executing user command: '" + commandStr + "'");
            isSuccessful = it->second->execute(commandParts);

            if (isSuccessful && commandName == "exit")
            {
                break;
            }
        }
        else
        {
            Logger::warning("Error: you entered invalid command '" + commandName + "'");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << '\n' << "Processing remaining server responses...";
    if (!app.processResponses())
    {
        std::cout << '\n' << "There are no new responses from the server" << '\n';
    }

    std::cout << "TelecomAppClient finished\n";
}