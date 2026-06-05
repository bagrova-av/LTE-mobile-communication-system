#pragma once
#ifndef COMMAND_INTERFACE_H
#define COMMAND_INTERFACE_H

#include <vector>
#include <string>

class CommandInterface
{
public:
    virtual bool execute(const std::vector<std::string>& args) = 0;

    virtual ~CommandInterface() = default;
};

#endif