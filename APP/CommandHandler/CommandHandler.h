#pragma once
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <vector>
#include <bits/stdc++.h>
#include <memory>

#include "../app/App.h"
#include "CommandInterface.h"

class CommandHandler
{
private:
    App& app;
    std::unordered_map<std::string, std::unique_ptr<CommandInterface>> commands;
    
    void printMenuHeader() const;

public:
    CommandHandler(App& app);

    void process();
};

#endif