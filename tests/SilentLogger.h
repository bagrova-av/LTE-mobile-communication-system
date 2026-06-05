#pragma once
#ifndef SILENT_LOGGER_H
#define SILENT_LOGGER_H

#include "../COMMON/logging/LoggerInterface.h"

class SilentLogger : public LoggerInterface
{
public:
    void log(LogLevel level, std::string_view message) override { }
};

#endif