#pragma once
#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include <vector>
#include <format>

#include "../APP/app/StartupParams.h"
#include "../APP/app/AppParams.h"
#include "./logging/Logger.h"
#include "../CORE/BaseStationConfig.h"
#include "../CORE/EpcConfig.h"

std::optional<StartupParams> parseCLI(int argc, char** argv, std::string& configPath);

bool parseJSON(const std::string& configPath, StartupParams& appConfig);

template<typename Point>
std::optional<Point> parseLocationImpl(const std::vector<typename Point::valueType>& pointComponents)
{
    size_t countComponents = pointComponents.size();
    if (countComponents == 0 || pointComponents.size() > Point::COUNT_COMPONENTS_IN_POINT)
    {
        Logger::error(std::format("Validation error: the location point must have {} components (got {})",
                                    Point::COUNT_COMPONENTS_IN_POINT, pointComponents.size()));
        return std::nullopt;
    }

    Point point;
    point.updateFromVector(pointComponents);
    return point;
}

std::optional<LocationPoint3D> parseLocation(const std::vector<CoordinatesType>& pointComponents);

std::optional<AppConfig> buildFinalConfig(const StartupParams& params);

std::vector<std::string> splitCommand(const std::string& commandStr);

int32_t parsePort(const std::string& portStr);

std::optional<EpcConfig> parseEpcJSON(const std::string& configPath);

std::optional<std::vector<BaseStationConfig>> parseBaseStationsJSON(const std::string& configPath);

#endif