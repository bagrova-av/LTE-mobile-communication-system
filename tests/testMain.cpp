#include <gtest/gtest.h>

#include <iostream>

#include "../COMMON/logging/Logger.h"
#include "SilentLogger.h"

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    
    Logger::setLogger(std::make_unique<SilentLogger>());
    
    std::streambuf* orig_cerr = std::cerr.rdbuf(nullptr);

    int result = RUN_ALL_TESTS();

    std::cerr.rdbuf(orig_cerr);
    
    return result;
}