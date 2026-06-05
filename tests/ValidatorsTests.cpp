#include <gtest/gtest.h>

#include "../COMMON/validationUtils.h"

TEST(ValidatorsTest, IMEI_ValidInput)
{
    EXPECT_TRUE(validateIMEI("490154203237518"));
    EXPECT_TRUE(validateIMEI("865434044201239"));
    EXPECT_TRUE(validateIMEI("353914100261218"));
}

TEST(ValidatorsTest, IMSI_ValidInput)
{
    EXPECT_TRUE(validateIMSI("310150123456789"));
    EXPECT_TRUE(validateIMSI("31015012345678"));
    EXPECT_TRUE(validateIMSI("3101501234567"));
    EXPECT_TRUE(validateIMSI("310150123456"));
    EXPECT_TRUE(validateIMSI("31015012345"));
    EXPECT_TRUE(validateIMSI("3101501234"));
    EXPECT_TRUE(validateIMSI("310150123"));
    EXPECT_TRUE(validateIMSI("31015012"));
    EXPECT_TRUE(validateIMSI("3101501"));
    EXPECT_TRUE(validateIMSI("310150"));
}

TEST(ValidatorsTest, IMEI_InvalidInput)
{
    EXPECT_FALSE(validateIMEI("4901542032375189"));
    EXPECT_FALSE(validateIMEI("49015420323751891"));

    EXPECT_FALSE(validateIMEI("49015420323751"));
    EXPECT_FALSE(validateIMEI("4901542032375"));
    EXPECT_FALSE(validateIMEI(""));

    EXPECT_FALSE(validateIMEI("49015420323751a"));
    EXPECT_FALSE(validateIMEI("-49015420323751"));
    EXPECT_FALSE(validateIMEI("49015420l237518"));
    EXPECT_FALSE(validateIMEI("865434@44201239"));
    EXPECT_FALSE(validateIMEI("35.914100261218"));
}

TEST(ValidatorsTest, IMSI_InvalidInput)
{
    EXPECT_FALSE(validateIMSI("31015"));
    EXPECT_FALSE(validateIMSI("3101"));
    EXPECT_FALSE(validateIMSI("310"));
    EXPECT_FALSE(validateIMSI("31"));
    EXPECT_FALSE(validateIMSI("3"));
    EXPECT_FALSE(validateIMSI(""));
    EXPECT_FALSE(validateIMSI("3101501234567898"));
    EXPECT_FALSE(validateIMSI("3101501234567890"));
    EXPECT_FALSE(validateIMSI("31015012345678901"));
    EXPECT_FALSE(validateIMSI("31015012345678a"));
    EXPECT_FALSE(validateIMSI("-31015012345678"));
    EXPECT_FALSE(validateIMSI("310150l23456789"));
    EXPECT_FALSE(validateIMSI("31015@123456789"));
    EXPECT_FALSE(validateIMSI("31.150123456789"));
}

TEST(ValidatorsTest, Port_ValidInput)
{
    EXPECT_TRUE(validatePort(1024));
    EXPECT_TRUE(validatePort(5000));
    EXPECT_TRUE(validatePort(65535));
    EXPECT_TRUE(validatePort(32768));
    EXPECT_TRUE(validatePort(49152));
}

TEST(ValidatorsTest, Port_InvalidInput)
{
    EXPECT_FALSE(validatePort(1023));
    EXPECT_FALSE(validatePort(1022));
    EXPECT_FALSE(validatePort(1000));
    EXPECT_FALSE(validatePort(1));
    EXPECT_FALSE(validatePort(0));
    EXPECT_FALSE(validatePort(-1));
    EXPECT_FALSE(validatePort(-100));
    EXPECT_FALSE(validatePort(-32768));

    EXPECT_FALSE(validatePort(65536));
    EXPECT_FALSE(validatePort(70000));
    EXPECT_FALSE(validatePort(100000));
    EXPECT_FALSE(validatePort(2147483647));
}