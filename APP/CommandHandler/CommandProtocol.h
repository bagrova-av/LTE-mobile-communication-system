#pragma once
#ifndef COMMAND_PROTOCOL_H
#define COMMAND_PROTOCOL_H

#include "SingleArgCommand.h"
#include "../app/App.h"

class CommandProtocol : public SingleArgCommand
{
private:
    App& app;

protected:
    virtual bool processValue(const std::string& value) override;

public:
    CommandProtocol(App& app) :
        SingleArgCommand("PROTOCOL", "BINARY/JSON"),
        app(app)
    {};
};

#endif
