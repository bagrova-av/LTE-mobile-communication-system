#pragma once
#ifndef EPC_CONFIG_H
#define EPC_CONFIG_H

#include <chrono>
#include <string>

struct HlrEirConfig
{
    std::string format;
    std::string provider;
    std::string path;
};

struct EpcConfig
{
    std::chrono::seconds simtelTtlSec;
    HlrEirConfig hlrEir;
    std::string nodesConfigFile;
    std::string mmeConfigFile;
    std::string smsCenterConfigFile;
};

#endif