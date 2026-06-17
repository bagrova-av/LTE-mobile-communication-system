#include "Logger.h"
#include "../ConsoleProvider.h"

#include <iostream>

std::unique_ptr<LoggerInterface> Logger::currentLogger = nullptr;
LogLevel Logger::minLevel = LogLevel::INFO;

void Logger::setLogger(std::unique_ptr<LoggerInterface> logger)
{
    currentLogger = std::move(logger);
}

void Logger::setMinimumLevel(LogLevel level)
{
    minLevel = level;
}

void Logger::log(LogLevel level, std::string_view message)
{
    if (currentLogger && level >= minLevel)
    {
        currentLogger->log(level, message);
    }

    if (level == LogLevel::ERROR || level == LogLevel::WARNING)
    {
        ConsoleProvider::executeSafePrint([message]()
                                        {
                                            std::cerr << message << '\n';
                                        }
        );
        //std::cerr << message << '\n';
    }
}

void Logger::info(std::string_view message)
{
    log(LogLevel::INFO, message);
}
void Logger::warning(std::string_view message)
{
    log(LogLevel::WARNING, message);
}
void Logger::error(std::string_view message)
{
    log(LogLevel::ERROR, message);
}
void Logger::debug(std::string_view message)
{
    log(LogLevel::DEBUG, message);
}