#include <gtest/gtest.h>

#include "testsHelper.h"

TEST(AppTest, AppInitialization)
{
    App app = createTestApp();

    IPv4 ip{127,0,0,1};
    LocationPoint3D point{0,0.6,-45.84};

    EXPECT_EQ(app.getPort(), 8080);
    EXPECT_EQ(app.getIP(), ip);
    EXPECT_EQ(app.getIMSI(), "404685505601234");
    EXPECT_EQ(app.getIMEI(), "490154203237518");
    EXPECT_EQ(app.getLocation(), point);
    EXPECT_EQ(app.getProtocol(), Protocol::JSON);
    EXPECT_EQ(app.getStatus(), false);
}

TEST(AppTest, SetActive)
{
    App app = createTestApp();
    
    // Checking the initial state
    EXPECT_FALSE(app.getStatus());

    app.setStatus(true);
    EXPECT_TRUE(app.getStatus());

    app.setStatus(false);
    EXPECT_FALSE(app.getStatus());
}

TEST(AppTest, SetProtocol)
{
    App app = createTestApp();
    
    // Checking the initial state
    EXPECT_EQ(app.getProtocol(), Protocol::JSON);

    app.setProtocol(Protocol::BINARY);
    EXPECT_EQ(app.getProtocol(), Protocol::BINARY);

    app.setProtocol(Protocol::JSON);
    EXPECT_EQ(app.getProtocol(), Protocol::JSON);
}

TEST(AppTest, SetLocation)
{
    App app = createTestApp();
    
    // Update only coordinate X
    LocationFlags flags = {1, 0, 0}; 
    app.setLocation(LocationPoint3D{10.05, 99.0, 99.0}, flags);
    
    EXPECT_FLOAT_EQ(app.getLocation().x, 10.05);
    EXPECT_FLOAT_EQ(app.getLocation().y, 0.6);
    EXPECT_FLOAT_EQ(app.getLocation().z, -45.84);

    // Update only coordinate Y
    flags.updateX = 0;
    flags.updateY = 1;
    flags.updateZ = 0;
    app.setLocation(LocationPoint3D{99.0, -34.567, 99.0}, flags);
    
    EXPECT_FLOAT_EQ(app.getLocation().x, 10.05);
    EXPECT_FLOAT_EQ(app.getLocation().y, -34.567);
    EXPECT_FLOAT_EQ(app.getLocation().z, -45.84);

    // Update only coordinate Z
    flags.updateX = 0;
    flags.updateY = 0;
    flags.updateZ = 1;
    app.setLocation(LocationPoint3D{99.0, 99.0, -0.0902}, flags);
    
    EXPECT_FLOAT_EQ(app.getLocation().x, 10.05);
    EXPECT_FLOAT_EQ(app.getLocation().y, -34.567);
    EXPECT_FLOAT_EQ(app.getLocation().z, -0.0902);

    // Update coordinates X and Y
    flags.updateX = 1;
    flags.updateY = 1;
    flags.updateZ = 0;
    app.setLocation(LocationPoint3D{-5.5, 20.31, 99.0}, flags);
    
    EXPECT_FLOAT_EQ(app.getLocation().x, -5.5);
    EXPECT_FLOAT_EQ(app.getLocation().y, 20.31);
    EXPECT_FLOAT_EQ(app.getLocation().z, -0.0902);

    // Update coordinates X and Z
    flags.updateX = 1;
    flags.updateY = 0;
    flags.updateZ = 1;
    app.setLocation(LocationPoint3D{49.501, -99.9, 137.978}, flags);
    
    EXPECT_FLOAT_EQ(app.getLocation().x, 49.501);
    EXPECT_FLOAT_EQ(app.getLocation().y, 20.31);
    EXPECT_FLOAT_EQ(app.getLocation().z, 137.978);

    // Update coordinates Y and Z
    flags.updateX = 0;
    flags.updateY = 1;
    flags.updateZ = 1;
    app.setLocation(LocationPoint3D{-95.0, -7.0, -3.00001}, flags);
    
    EXPECT_FLOAT_EQ(app.getLocation().x, 49.501);
    EXPECT_FLOAT_EQ(app.getLocation().y, -7.0);
    EXPECT_FLOAT_EQ(app.getLocation().z, -3.00001);

    // Update coordinates X, Y and Z
    flags.updateX = 1;
    flags.updateY = 1;
    flags.updateZ = 1;
    app.setLocation(LocationPoint3D{-39.163, 57.9003, 642.80009}, flags);
    
    EXPECT_FLOAT_EQ(app.getLocation().x, -39.163);
    EXPECT_FLOAT_EQ(app.getLocation().y, 57.9003);
    EXPECT_FLOAT_EQ(app.getLocation().z, 642.80009);
}