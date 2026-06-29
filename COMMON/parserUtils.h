#pragma once
#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include <vector>
#include <format>
#include <map>

#include "../APP/app/StartupParams.h"
#include "../APP/app/AppParams.h"
#include "./logging/Logger.h"
#include "../CORE/entitiesConfigs/BaseStationConfig.h"
#include "../CORE/entitiesConfigs/EpcConfig.h"
#include "../CORE/entitiesConfigs/SubscriberRecord.h"
#include "../CORE/entitiesConfigs/MmeConfig.h"
#include "../CORE/entitiesConfigs/SmsCenterConfig.h"
#include "../CORE/entitiesConfigs/AddressBookEntry.h"

std::optional<StartupParams> parseCLI(int argc, char** argv, std::string& configPath);

bool parseAppJSON(const std::string& configPath, StartupParams& appConfig);

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

bool parseEpcJSON(const std::string& configPath, EpcConfig& epcConfig);

bool parseBaseStationsJSON(const std::string& configPath, std::vector<BaseStationConfig>& bsConfigs);

bool parseMmeJSON(const std::string& configPath, std::vector<MmeConfig>& mmes);

bool parseSmsCenterJSON(const std::string& configPath, SmsCenterConfig& smsCenterConfig);

bool parseAddressBook(const std::string& configPath, const IMSI& myImsi,
                      std::map<size_t, AddressBookEntry>& addressBook);

#endif