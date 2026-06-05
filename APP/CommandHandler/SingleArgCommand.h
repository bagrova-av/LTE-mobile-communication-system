#pragma once
#ifndef SINGLE_ARG_COMMAND_H
#define SINGLE_ARG_COMMAND_H

#include "CommandInterface.h"
#include "../../COMMON/utils.h"
#include "../../COMMON/logging/Logger.h"

class SingleArgCommand : public CommandInterface
{
private:
    std::string commandName;
    std::string allowedValuesMessage;

protected:
    virtual bool processValue(const std::string& value) = 0;

public:
    SingleArgCommand(const std::string& commandName, const std::string& allowedValuesMessage) :
        commandName(commandName),
        allowedValuesMessage(allowedValuesMessage)
    {};

    virtual bool execute(const std::vector<std::string>& args) override;
};

#endif