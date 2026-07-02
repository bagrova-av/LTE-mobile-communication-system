#pragma once
#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <chrono>

bool setSocketTimeout(int fileDescriptor, std::chrono::microseconds microseconds);

#endif