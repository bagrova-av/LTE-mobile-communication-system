#include "utils.h"
#include "./logging/Logger.h"
#include "./logging/FileLogger.h"

#include <filesystem>
#include <iostream>
#include <format>

#include <algorithm>
#include <chrono>

std::string toLowerCase(const std::string& str)
{
    std::string lowerStr = str;
    std::transform(str.cbegin(), str.cend(), lowerStr.begin(), ::tolower);
    return lowerStr;
}

std::string currentDateTimeToString()
{
    auto now = std::chrono::system_clock::now();
    auto nowSeconds = std::chrono::floor<std::chrono::seconds>(now);
    auto timeLocal = std::chrono::zoned_time{std::chrono::current_zone(), nowSeconds};
    return std::format("{:%d.%m.%Y %H:%M:%S}", timeLocal);
}

void initLogger(const std::string& executablePath, const std::string& fileName)
{
    std::filesystem::path execPath = executablePath;
    std::string logFileName = fileName + std::to_string(getpid()) + ".log";
    std::filesystem::path logPath = execPath.parent_path() / logFileName;

    auto fileLogger = std::make_unique<FileLogger>(logPath.string());
    Logger::setLogger(std::move(fileLogger));

    std::cout << "==========================================" << '\n';
    std::cout << "  Log file: " << logFileName << '\n';
    std::cout << "  Process ID (PID): " << getpid() << '\n';
    std::cout << "==========================================" << '\n';
}