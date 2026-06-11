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
    std::optional<M_TMSI> m_tmsi;
};

struct AppConfig
{
    NetworkAddress netAddr;
    IMSI imsi;
    IMEI imei;
    MSISDN msisdn;
    Nodes nodes;

    AppConfig() = default;
    AppConfig(const NetworkAddress& addr,
              const IMSI& imsi, const IMEI& imei, const MSISDN& msisdn,
              const Nodes& nodes);
    AppConfig(const NetworkAddress& addr,
              const IMSI& imsi, const IMEI& imei, const MSISDN& msisdn);
    AppConfig(const AppConfig& other);
    AppConfig(AppConfig&& other) noexcept;

    AppConfig& operator=(const AppConfig& other);
    AppConfig& operator=(AppConfig&& other) noexcept;
};

#endif