#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <optional>
#include <stdexcept>
#include <type_traits>

template <typename T>
std::optional<T> convertFromString(const std::string& str)
{
    if (str.empty())
    {
        return std::nullopt;
    }

    T result;
    size_t processedPos = 0;
    try
    {
        if constexpr (std::is_same_v<T, float>)
        {
            result = std::stof(str, &processedPos);
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            result = std::stod(str, &processedPos);
        }
        else if constexpr (std::is_same_v<T, long double>)
        {
            result = std::stold(str, &processedPos);
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            result = std::stoi(str, &processedPos);
        }
        else if constexpr (std::is_same_v<T, long>)
        {
            result = std::stol(str, &processedPos);
        }
        else if constexpr (std::is_same_v<T, long long>)
        {
            result = std::stoll(str, &processedPos);
        }
        else if constexpr (std::is_same_v<T, unsigned long>)
        {
            result = std::stoul(str, &processedPos);
        }
        else if constexpr (std::is_same_v<T,  unsigned long long>)
        {
            result = std::stoull(str, &processedPos);
        }
        else
        {
            return std::nullopt;
        }

        if (processedPos != str.length())
        {
            return std::nullopt;
        }

        return result;
    }
    catch (const std::exception& ex)
    {
        return std::nullopt;
    }
}

std::string toLowerCase(const std::string& str);

std::string currentDateTimeToString();

bool isBlankString(const std::string& str);

void initLogger(const std::string& executablePath, const std::string& fileName);

#endif