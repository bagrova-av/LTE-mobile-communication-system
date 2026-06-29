#pragma once
#ifndef COMMAND_RECEIVED_SMS_H
#define COMMAND_RECEIVED_SMS_H

#include "CommandInterface.h"
#include "../app/App.h"

class CommandReceivedSms : public CommandInterface
{
private:
    App& app;

public:
    CommandReceivedSms(App& app) :
        app(app) {};

    virtual bool execute(const std::vector<std::string>& args) override;
};

#endif