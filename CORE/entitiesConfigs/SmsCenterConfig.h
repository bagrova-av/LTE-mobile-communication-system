#pragma once
#ifndef SMS_CENTER_CONFIG_H
#define SMS_CENTER_CONFIG_H

#include <chrono>

struct SmsCenterConfig
{
    size_t maxTextLength;
    size_t maxBufferCount;
    std::chrono::milliseconds smsTtlMs;
};

#endif