#include "FileLogger.h"
#include "../utils.h"

#include <iostream>

#include <thread>

FileLogger::FileLogger(const std::string& fileName)
{
    logFile.open(fileName, std::ios::app);
    if (!logFile.is_open())
    {
        std::cerr << "Error: Could not open log file" << std::endl;
    }
}

FileLogger::~FileLogger()
{
    if (logFile.is_open())
    {
        logFile.close();
    }
}

void FileLogger::log(LogLevel level, std::string_view message)
{
    std::lock_guard<std::mutex> lock(logMutex);

    if (logFile.is_open())
    {
        logFile << '[' << currentDateTimeToString() << "] " 
                << "[Thread-" << std::this_thread::get_id() << "] "
                << '[' << levelToString(level) << "] " 
                << message << '\n';
        logFile.flush();
    }
}