#include <gtest/gtest.h>

#include "../COMMON/parserUtils.h"
#include "../CORE/types.h"
#include "testsHelper.h"

TEST(ParserTest, ParseLocation_ValidComponents)
{
    // Only component X
    auto loc1 = parseLocation({5.501});
    ASSERT_TRUE(loc1.has_value());
    EXPECT_FLOAT_EQ(loc1.value().x, 5.501);
    EXPECT_FLOAT_EQ(loc1.value().y, 0.0);
    EXPECT_FLOAT_EQ(loc1.value().z, 0.0);

    // X, Y components
    auto loc2 = parseLocation({1.00001, -2.09});
    ASSERT_TRUE(loc2.has_value());
    EXPECT_FLOAT_EQ(loc2.value().x, 1.00001);
    EXPECT_FLOAT_EQ(loc2.value().y, -2.09);
    EXPECT_FLOAT_EQ(loc2.value().z, 0.0);

    // X, Y, Z components
    auto loc3 = parseLocation({-0.01, 56.3901, -23.56});
    ASSERT_TRUE(loc3.has_value());
    EXPECT_FLOAT_EQ(loc3.value().x, -0.01);
    EXPECT_FLOAT_EQ(loc3.value().y, 56.3901);
    EXPECT_FLOAT_EQ(loc3.value().z, -23.56);
}

TEST(ParserTest, ParseLocation_InvalidComponents)
{
    EXPECT_FALSE(parseLocation({}).has_value());
    EXPECT_FALSE(parseLocation({1, 2, 3, 4}).has_value());
    EXPECT_FALSE(parseLocation({1, 2, 3, 4, 5}).has_value());
}

TEST(ParserTest, BuildFinalConfig_Valid)
{
    StartupParams params;
    params.ip = "192.168.5.1";
    params.port = "8080";
    params.imei = "490154203237518";
    params.imsi = "404685505601234";
    params.initialLocation = LocationPoint3D{134.001, 21.34, -3.000564};

    auto config = buildFinalConfig(params);
    EXPECT_TRUE(config.has_value());
}

TEST(ParserTest, BuildFinalConfig_MissingParams)
{
    StartupParams params;
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.ip = "192.168.5.1";
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.port = "8080";
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.imei = "490154203237518";
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.imsi = "404685505601234";
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.imsi = std::nullopt;
    params.initialLocation = LocationPoint3D{134.001, 21.34, -3.000564};
    EXPECT_FALSE(buildFinalConfig(params).has_value());
}

TEST(ParserTest, BuildFinalConfig_InvalidNetworkAddress)
{
    StartupParams params;
    params.ip = "192.168.1.0";
    params.port = "1025";
    params.imei = "490154203237518";
    params.imsi = "404685505601234";
    params.initialLocation = LocationPoint3D{1, 2, 3};
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.ip = "192.168.1.254";
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.ip = "192.168.256.253";
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.ip = "192.168.1.1";
    params.port = "1023";
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.port = "65536";
    EXPECT_FALSE(buildFinalConfig(params).has_value());

    params.port = "65l53";
    EXPECT_FALSE(buildFinalConfig(params).has_value());
}

TEST(ParserTest, ParseCLI_ValidMinimal)
{
    std::vector<std::string> args = {"./TelecomApp", "-a", "192.168.1.1", "-p", "8080"};
    auto argv = createArgv(args);
    std::string configPath = "";

    auto params = parseCLI(args.size(), argv.data(), configPath);
    
    ASSERT_TRUE(params.has_value());
    EXPECT_EQ(params.value().ip.value(), "192.168.1.1");
    EXPECT_EQ(params.value().port.value(), "8080");
}

TEST(ParserTest, ParseCLI_Valid)
{
    std::string configPath = "";
    std::vector<std::string> args = {"./TelecomApp", "--config", "../data/config.json",
                                                    "--ip", "127.0.5.1",
                                                    "--port", "7777",
                                                    "--imsi", "404685505601234",
                                                    "--imei", "490154203237518",
                                                    "--loc", "0", "0.6", "-45.84",
                                                    "--nodes", "nodeA,nodeB" };
    auto argv = createArgv(args);
    auto params = parseCLI(args.size(), argv.data(), configPath);

    LocationPoint3D point{0, 0.6, -45.84};
    ASSERT_TRUE(params.has_value());
    EXPECT_EQ(configPath, "../data/config.json");
    EXPECT_EQ(params.value().ip.value(), "127.0.5.1");
    EXPECT_EQ(params.value().port.value(), "7777");
    EXPECT_EQ(params.value().imsi.value(), "404685505601234");
    EXPECT_EQ(params.value().imei.value(), "490154203237518");
    EXPECT_EQ(params.value().initialLocation.value(), point);
    EXPECT_EQ(params.value().nodes, "nodeA,nodeB");

    args = {"./TelecomApp", "--config", "../data/config1.json",
                            "--ip", "127.12.5.1",
                            "--port", "8080",
                            "--imsi", "504685505601234",
                            "--imei", "590154203237518",
                            "--loc", "0.3", "-34.6", "-045.84",
                            "--nodes", "nodeC,nodeD" };
    argv = createArgv(args);
    params = parseCLI(args.size(), argv.data(), configPath);
    point.x = 0.3;
    point.y = -34.6;
    point.z = -45.84;
    ASSERT_TRUE(params.has_value());
    EXPECT_EQ(configPath, "../data/config1.json");
    EXPECT_EQ(params.value().ip.value(), "127.12.5.1");
    EXPECT_EQ(params.value().port.value(), "8080");
    EXPECT_EQ(params.value().imsi.value(), "504685505601234");
    EXPECT_EQ(params.value().imei.value(), "590154203237518");
    EXPECT_EQ(params.value().initialLocation.value(), point);
    EXPECT_EQ(params.value().nodes, "nodeC,nodeD");
}

TEST(ParserTest, ParseCLI_UnknownArgument)
{
    std::vector<std::string> args = {"./TelecomApp", "-a", "192.168.1.1", "--unknown", "123"};
    auto argv = createArgv(args);
    std::string configPath = "";

    auto params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "-a", "192.168.1.1", "smth", "interesting"};
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "-a", "192.168.1.1", "--p0rt", "8080"};
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "-a", "192.168.1.1", "--!port", "8080"};
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "-a", "192.168.1.1", "--port2", "8080"};
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "-a", "192.168.1.1", "--location", "1", "2", "3"};
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "-a", "192.168.1.1", "-r", "8080"};
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());
}

TEST(ParserTest, ParseCLI_MissingArgumentValue)
{
    std::vector<std::string> args = {"./TelecomApp", "-p"};
    auto argv = createArgv(args);
    std::string configPath = "";

    auto params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "-a", "192.168.1.1", "-l", "1"};
    argv = createArgv(args);
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "-a", "192.168.1.1", "-l", "1", "2"};
    argv = createArgv(args);
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());

    args = {"./TelecomApp", "--ip", "192.168.1.1", "-p", "8080", "--imsi", "--imei", "490154203237518"};
    argv = createArgv(args);
    params = parseCLI(args.size(), argv.data(), configPath);
    EXPECT_FALSE(params.has_value());
}

TEST(ParserTest, ParsePort_ValidPort)
{
    EXPECT_EQ(parsePort("8080"), 8080);
    EXPECT_EQ(parsePort("1024"), 1024);
    EXPECT_EQ(parsePort("65535"), 65535);
    EXPECT_EQ(parsePort("0"), 0);
    EXPECT_EQ(parsePort("1"), 1);
}

TEST(ParserTest, ParsePort_ValidPortWithLeadingZeros)
{
    EXPECT_EQ(parsePort("001024"), 1024);
    EXPECT_EQ(parsePort("0008080"), 8080);
    EXPECT_EQ(parsePort("00000"), 0);
}

TEST(ParserTest, ParsePort_InvalidPort)
{
    EXPECT_THROW(parsePort("abc"), std::invalid_argument);
    EXPECT_THROW(parsePort("port8080"), std::invalid_argument);
    EXPECT_THROW(parsePort("80x80"), std::invalid_argument);
    EXPECT_THROW(parsePort("12.34"), std::invalid_argument);
    EXPECT_THROW(parsePort(""), std::invalid_argument);
    EXPECT_THROW(parsePort(" "), std::invalid_argument);
    EXPECT_THROW(parsePort("8080 "), std::invalid_argument);
    EXPECT_THROW(parsePort("12l3"), std::invalid_argument);
    EXPECT_THROW(parsePort("1223r"), std::invalid_argument);
}

TEST(ParserTest, ParsePort_BoundaryExtremeValues)
{
    EXPECT_EQ(parsePort("2147483647"), 2147483647); // max(int32_t)
    EXPECT_THROW(parsePort("2147483648"), std::invalid_argument);
    EXPECT_THROW(parsePort("21474836470"), std::invalid_argument);
}