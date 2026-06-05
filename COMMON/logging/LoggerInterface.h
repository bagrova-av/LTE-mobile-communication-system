#pragma once
#ifndef LOGGER_INTERFACE_H 
#define LOGGER_INTERFACE_H

#include <string>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class LoggerInterface
{
public:
    virtual void log(LogLevel level, std::string_view message) = 0;

    virtual ~LoggerInterface() = default;
};

constexpr std::string_view levelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
    }
    return "UNKNOWN";
}

#endif