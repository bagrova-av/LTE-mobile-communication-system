#pragma once
#ifndef COMMAND_ACTIVE_H
#define COMMAND_ACTIVE_H

#include "SingleArgCommand.h"
#include "../app/App.h"

class CommandActive : public SingleArgCommand
{
private:
    App& app;

protected:
    virtual bool processValue(const std::string& value) override;

public:
    CommandActive(App& app) : 
        SingleArgCommand("ACTIVE", "false/true/0/1"),
        app(app)
    {};
};

#endif