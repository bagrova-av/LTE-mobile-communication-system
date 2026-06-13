#pragma once
#ifndef EPC_CONFIG_H
#define EPC_CONFIG_H

#include "../types.h"

struct HlrEirConfig
{
    std::string format;
    std::string provider;
    std::string path;
};

struct EpcConfig
{
    Seconds simtelTtl;
    HlrEirConfig hlrEir;
    std::string nodesConfigFile;
    std::string mmeConfigFile;
    std::string smsCenterConfigFile;
};

#endif