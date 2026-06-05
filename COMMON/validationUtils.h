#pragma once
#ifndef VALIDATION_UTILS_H
#define VALIDATION_UTILS_H

#include <string>

#include "../APP/app/StartupParams.h"

bool checkCorrectNameFile(const std::string& nameFile, const std::string& expectedExtension);

bool validatePort(int32_t port);

bool validateIMEI(const std::string& imei);

bool validateIMSI(const std::string& imsi);

bool validateServerConfig(const StartupParams& params);

#endif