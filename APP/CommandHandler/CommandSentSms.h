#pragma once
#ifndef COMMAND_SENT_SMS_H
#define COMMAND_SENT_SMS_H

#include "CommandInterface.h"
#include "../app/App.h"

class CommandSentSms : public CommandInterface
{
private:
    App& app;

    std::string covertIdsToString(const std::vector<SmsId>& ids);

public:
    CommandSentSms(App& app) :
        app(app) {};

    virtual bool execute(const std::vector<std::string>& args) override;
};

#endif