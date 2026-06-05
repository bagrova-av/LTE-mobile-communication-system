#pragma once
#ifndef APP_PARAMS_H
#define APP_PARAMS_H

#include <string>

#include "../../CORE/types.h"
#include "../../CORE/NetworkAddress.h"

struct AppState
{
    LocationPoint3D location;
    bool isActive = false;
    Protocol protocol = Protocol::JSON;
};

struct AppConfig
{
    NetworkAddress netAddr;
    IMSI imsi;
    IMEI imei;
    Nodes nodes;

    AppConfig() = default;
    AppConfig(const NetworkAddress& addr,
              const IMSI& imsi, const IMEI& imei,
              const Nodes& nodes);
    AppConfig(const NetworkAddress& addr,
              const IMSI& imsi, const IMEI& imei);
    AppConfig(const AppConfig& other);
    AppConfig(AppConfig&& other) noexcept;

    AppConfig& operator=(const AppConfig& other);
    AppConfig& operator=(AppConfig&& other) noexcept;
};

#endif