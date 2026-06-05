#include "utils.h"
#include "./logging/Logger.h"

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