#pragma once
#ifndef COMMAND_EXIT_H
#define COMMAND_EXIT_H

#include "CommandInterface.h"

class CommandExit : public CommandInterface
{
public:
    virtual bool execute(const std::vector<std::string>& args) override;
};

#endif