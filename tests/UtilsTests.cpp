#include <gtest/gtest.h>

#include "../COMMON/utils.h"

TEST(UtilsTest, ConvertFromString_ValidInput)
{
    auto result = convertFromString<float>("12.5");
    EXPECT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(result.value(), 12.5f);

    EXPECT_FLOAT_EQ(convertFromString<float>("-42.0").value(), -42.0);
    EXPECT_FLOAT_EQ(convertFromString<float>("0").value(), 0.0);

    EXPECT_EQ(convertFromString<int>("1011").value(), 1011);
    EXPECT_EQ(convertFromString<int>("-1011").value(), -1011);
}

TEST(UtilsTest, ConvertFromString_InvalidInput)
{
    EXPECT_FALSE(convertFromString<float>("abc").has_value());
    EXPECT_FALSE(convertFromString<float>("").has_value());
    EXPECT_FALSE(convertFromString<float>("L").has_value());
    EXPECT_FALSE(convertFromString<float>("a").has_value());
    EXPECT_FALSE(convertFromString<float>(" ").has_value());
    EXPECT_FALSE(convertFromString<float>("-").has_value());

    EXPECT_FALSE(convertFromString<int>("10l11").has_value());
}

TEST(UtilsTest, ConvertFromString_PartialInput)
{
    EXPECT_FALSE(convertFromString<float>("12.5abc").has_value());
    EXPECT_FALSE(convertFromString<float>("12.5a").has_value());
    EXPECT_FALSE(convertFromString<float>("12.5-").has_value());
    EXPECT_FALSE(convertFromString<float>("12l.5").has_value());
    EXPECT_FALSE(convertFromString<float>("12-5").has_value());
    EXPECT_FALSE(convertFromString<float>("22!6.5").has_value());
    EXPECT_FALSE(convertFromString<float>("w12.5").has_value());
}

std::string toLowerCase(const std::string& str);
TEST(UtilsTest, ToLowerCase)
{
    EXPECT_EQ(toLowerCase("12.5abc"), "12.5abc");
    EXPECT_EQ(toLowerCase("aCtIvE"), "active");
    EXPECT_EQ(toLowerCase("AcTiVe"), "active");
    EXPECT_EQ(toLowerCase("Active"), "active");
    EXPECT_EQ(toLowerCase("activE"), "active");
    EXPECT_EQ(toLowerCase("ACTIVE"), "active");
    EXPECT_EQ(toLowerCase("1"), "1");
    EXPECT_EQ(toLowerCase("."), ".");
    EXPECT_EQ(toLowerCase("JS0N"), "js0n");
    EXPECT_EQ(toLowerCase("ProTOCol-$Mth"), "protocol-$mth");
    EXPECT_EQ(toLowerCase("pRoToCoL0_0"), "protocol0_0");
}
