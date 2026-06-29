#include <gtest/gtest.h>

#include "../APP/CommandHandler/CommandActive.h"
#include "../APP/CommandHandler/CommandProtocol.h"
#include "../APP/CommandHandler/CommandMove.h"
#include "../APP/CommandHandler/CommandExit.h"
#include "testsHelper.h"

TEST(CommandsTest, CommandActive_Valid)
{
    App app = createTestApp();
    CommandActive command(app);

    EXPECT_TRUE(command.execute({"true"}));
    EXPECT_TRUE(app.getStatus());

    EXPECT_TRUE(command.execute({"false"}));
    EXPECT_FALSE(app.getStatus());

    EXPECT_TRUE(command.execute({"1"}));
    EXPECT_TRUE(app.getStatus());

    EXPECT_TRUE(command.execute({"0"}));
    EXPECT_FALSE(app.getStatus());
    
    EXPECT_TRUE(command.execute({"TrUe"}));
    EXPECT_TRUE(app.getStatus());

    EXPECT_TRUE(command.execute({"fAlSE"}));
    EXPECT_FALSE(app.getStatus());

    EXPECT_TRUE(command.execute({"TRUE"}));
    EXPECT_TRUE(app.getStatus());

    EXPECT_TRUE(command.execute({"FALSE"}));
    EXPECT_FALSE(app.getStatus());
}

TEST(CommandsTest, CommandActive_Invalid)
{
    App app = createTestApp();
    CommandActive command(app);

    EXPECT_FALSE(command.execute({}));
    EXPECT_FALSE(command.execute({"true", "false"}));
    EXPECT_FALSE(command.execute({"true", "false", "DS"}));

    EXPECT_FALSE(command.execute({"yes"}));
    EXPECT_FALSE(command.execute({""}));
    EXPECT_FALSE(command.execute({"-"}));
    EXPECT_FALSE(command.execute({"BYE"}));

    EXPECT_FALSE(command.execute({"trui"}));
    EXPECT_FALSE(command.execute({"true1"}));
    EXPECT_FALSE(command.execute({"tr1e"}));
    EXPECT_FALSE(command.execute({"f0lse"}));
    EXPECT_FALSE(command.execute({"fals"}));
    EXPECT_FALSE(command.execute({"folse"}));
}

TEST(CommandsTest, CommandMove_Valid)
{
    App app = createTestApp(); //LocationPoint3D{0,0.6,-45.84}
    CommandMove command(app);

    // Update only X component
    EXPECT_TRUE(command.execute({"10.5"}));
    EXPECT_FLOAT_EQ(app.getLocation().x, 10.5);
    EXPECT_FLOAT_EQ(app.getLocation().y, 0.6);
    EXPECT_FLOAT_EQ(app.getLocation().z, -45.84);

    // Update X and Y components
    EXPECT_TRUE(command.execute({"-53.00003", "-34.567"}));
    EXPECT_FLOAT_EQ(app.getLocation().x, -53.00003);
    EXPECT_FLOAT_EQ(app.getLocation().y, -34.567);
    EXPECT_FLOAT_EQ(app.getLocation().z, -45.84);

    // Update X, Y and Z components
    EXPECT_TRUE(command.execute({"134.001", "021.34", "-003.000564"}));
    EXPECT_FLOAT_EQ(app.getLocation().x, 134.001);
    EXPECT_FLOAT_EQ(app.getLocation().y, 21.34);
    EXPECT_FLOAT_EQ(app.getLocation().z, -3.000564);
}

TEST(CommandsTest, CommandMove_Invalid)
{
    App app = createTestApp();
    CommandMove command(app);

    EXPECT_FALSE(command.execute({}));
    EXPECT_FALSE(command.execute({"1", "2", "3", "4"}));
    EXPECT_FALSE(command.execute({"1", "2", "3", "4", "5"}));

    EXPECT_FALSE(command.execute({"abc"}));
    EXPECT_FALSE(command.execute({"-"}));
    EXPECT_FALSE(command.execute({"1.l5"}));
    EXPECT_FALSE(command.execute({"@1.5"}));
    EXPECT_FALSE(command.execute({"1.5w"}));
    EXPECT_FALSE(command.execute({"1-5"}));

    EXPECT_FALSE(command.execute({"1.0", "xyz"}));
    EXPECT_FALSE(command.execute({"1.0", "34.2l"}));
    EXPECT_FALSE(command.execute({"1.5t6", "34.2"}));
    EXPECT_FALSE(command.execute({"1.0", "34.r2"}));
    EXPECT_FALSE(command.execute({"1.0", "q34.2"}));
    EXPECT_FALSE(command.execute({"p1.0", "34.2"}));
    EXPECT_FALSE(command.execute({"1.0t", "34.2"}));

    EXPECT_FALSE(command.execute({"1.0", "34.2", "o23.45"}));
    EXPECT_FALSE(command.execute({"1.0", "34.2", "23.45-"}));
    EXPECT_FALSE(command.execute({"1.0", "34.2", "23.4-5"}));
    EXPECT_FALSE(command.execute({"1.0", "34.2", "2g3.45"}));
    EXPECT_FALSE(command.execute({"1.0", "34.2", "23:45"}));
    EXPECT_FALSE(command.execute({"1.0", "34.l2", "23.45"}));
    EXPECT_FALSE(command.execute({"1.0", "34.2w", "23.45"}));
    EXPECT_FALSE(command.execute({"1.0", "{34.2", "23.45"}));
    EXPECT_FALSE(command.execute({"1.9x", "34.2", "23.45"}));
    EXPECT_FALSE(command.execute({"1.z9", "34.2", "23.45"}));
    EXPECT_FALSE(command.execute({"!1.89", "34.2", "23.45"}));
}

TEST(CommandsTest, CommandProtocol_Valid)
{
    App app = createTestApp();
    CommandProtocol command(app);

    EXPECT_TRUE(command.execute({"binary"}));
    EXPECT_EQ(app.getProtocol(), Protocol::BINARY);

    EXPECT_TRUE(command.execute({"json"}));
    EXPECT_EQ(app.getProtocol(), Protocol::JSON);

    EXPECT_TRUE(command.execute({"bInArY"}));
    EXPECT_EQ(app.getProtocol(), Protocol::BINARY);

    EXPECT_TRUE(command.execute({"JsOn"}));
    EXPECT_EQ(app.getProtocol(), Protocol::JSON);

    EXPECT_TRUE(command.execute({"BINARY"}));
    EXPECT_EQ(app.getProtocol(), Protocol::BINARY);

    EXPECT_TRUE(command.execute({"JSON"}));
    EXPECT_EQ(app.getProtocol(), Protocol::JSON);
}

TEST(CommandsTest, CommandProtocol_Invalid)
{
    App app = createTestApp();
    CommandProtocol command(app);

    EXPECT_FALSE(command.execute({}));
    EXPECT_FALSE(command.execute({"json", "txt"}));
    EXPECT_FALSE(command.execute({"pdf", "docx", "mp3"}));

    EXPECT_FALSE(command.execute({"txt"}));
    EXPECT_FALSE(command.execute({""}));
    EXPECT_FALSE(command.execute({"-"}));
    EXPECT_FALSE(command.execute({"MP4"}));

    EXPECT_FALSE(command.execute({"js0n"}));
    EXPECT_FALSE(command.execute({"jsan"}));
    EXPECT_FALSE(command.execute({"gson"}));
    EXPECT_FALSE(command.execute({"jso"}));
    EXPECT_FALSE(command.execute({"son"}));
    EXPECT_FALSE(command.execute({"json1"}));
    EXPECT_FALSE(command.execute({"@json"}));
    EXPECT_FALSE(command.execute({"j!on"}));
    EXPECT_FALSE(command.execute({"bin"}));
    EXPECT_FALSE(command.execute({"binari"}));
    EXPECT_FALSE(command.execute({"b!nary"}));
    EXPECT_FALSE(command.execute({"*binary"}));
    EXPECT_FALSE(command.execute({"binary~"}));
    EXPECT_FALSE(command.execute({"binory"}));
}

TEST(CommandsTest, CommandExit_Valid)
{
    CommandExit command;
    EXPECT_TRUE(command.execute({}));
}

TEST(CommandsTest, CommandExit_Invalid)
{
    CommandExit command;
    EXPECT_FALSE(command.execute({""}));
    EXPECT_FALSE(command.execute({"wewe"}));
    EXPECT_FALSE(command.execute({"0"}));
    EXPECT_FALSE(command.execute({"1"}));
    EXPECT_FALSE(command.execute({"1", "0"}));
    EXPECT_FALSE(command.execute({"1", "0", "-1"}));
}