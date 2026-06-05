#pragma once
#ifndef COMMAND_MOVE_H
#define COMMAND_MOVE_H

#include "CommandInterface.h"
#include "../app/App.h"
#include "../../CORE/types.h"
#include "../../COMMON/utils.h"
#include "../../COMMON/logging/Logger.h"

class CommandMove : public CommandInterface
{
private:
    App& app;

public:
    CommandMove(App& app) :
        app(app) {};

    virtual bool execute(const std::vector<std::string>& args) override;
};

#endif