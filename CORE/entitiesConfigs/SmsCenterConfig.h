#pragma once
#ifndef SMS_CENTER_CONFIG_H
#define SMS_CENTER_CONFIG_H

#include "../types.h"

struct SmsCenterConfig
{
    size_t maxTextLength;
    size_t maxBufferCount;
    Milliseconds ttlMs;
};

#endif