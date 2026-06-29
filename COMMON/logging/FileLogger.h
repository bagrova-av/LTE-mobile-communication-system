#pragma once
#ifndef FILE_LOGGER_H 
#define FILE_LOGGER_H

#include <fstream>
#include <mutex>

#include "LoggerInterface.h"

class FileLogger : public LoggerInterface
{
private:
    std::ofstream logFile;
    std::mutex logMutex;

public:
    FileLogger(const std::string& fileName);
    ~FileLogger();

    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;

    FileLogger(FileLogger&&) = delete;
    FileLogger& operator=(FileLogger&&) = delete;

    virtual void log(LogLevel level, std::string_view message) override;
};

#endif