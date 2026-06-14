#pragma once
#ifndef STARTUP_PARAMS_H
#define STARTUP_PARAMS_H

#include <optional>

#include "../../CORE/types.h"

struct StartupParams
{
    std::optional<std::string> ip;
    std::optional<std::string> port;
    std::optional<std::string> imsi;
    std::optional<std::string> imei;
    std::optional<std::string> msisdn;
    std::optional<LocationPoint3D> initialLocation;
    std::string addressBookPath;

    std::string nodes = "";

    StartupParams() = default;
    StartupParams(const StartupParams& other);
    StartupParams(StartupParams&& other) noexcept;

    StartupParams& operator=(const StartupParams& other);
    StartupParams& operator=(StartupParams&& other) noexcept;
};

#endif