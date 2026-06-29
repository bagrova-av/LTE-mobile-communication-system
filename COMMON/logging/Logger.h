#pragma once
#ifndef LOGGER_H 
#define LOGGER_H

#include <memory>

#include "LoggerInterface.h"

class Logger
{
private:
    static std::unique_ptr<LoggerInterface> currentLogger;
    static LogLevel minLevel;

public:
    static void setLogger(std::unique_ptr<LoggerInterface> logger);
    static void setMinimumLevel(LogLevel level);

    static void log(LogLevel level, std::string_view message);

    static void info(std::string_view message);
    static void warning(std::string_view message);
    static void error(std::string_view message);
    static void debug(std::string_view message);
};

#endif