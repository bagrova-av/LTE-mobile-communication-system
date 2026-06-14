#pragma once
#ifndef COMMAND_SMS_H
#define COMMAND_SMS_H

#include "CommandInterface.h"
#include "../app/App.h"

class CommandSms : public CommandInterface
{
private:
    App& app;

public:
    CommandSms(App& app) :
        app(app) {};

    virtual bool execute(const std::vector<std::string>& args) override;
};

#endif