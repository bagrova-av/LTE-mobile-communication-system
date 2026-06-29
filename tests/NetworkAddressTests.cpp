#include <gtest/gtest.h>

#include "../CORE/NetworkAddress.h"

// Tests for NetworkAddress(int64_t ipAddress, int32_t port)
TEST(NetworkAddressTest, IntConstructor_Valid)
{
    // 3232236801 = 192.168.5.1
    NetworkAddress addr(3232236801, 8080);
    
    EXPECT_EQ(addr.getPort(), 8080);
    EXPECT_EQ(addr.getIP().octets[0], 192);
    EXPECT_EQ(addr.getIP().octets[1], 168);
    EXPECT_EQ(addr.getIP().octets[2], 5);
    EXPECT_EQ(addr.getIP().octets[3], 1);
}

TEST(NetworkAddressTest, IntConstructor_ValidEdgeCases)
{
    // Port: lower bound (1024) and upper bound (65535)
    EXPECT_NO_THROW(NetworkAddress(3232236801, 1024));
    EXPECT_NO_THROW(NetworkAddress(3232236801, 65535));

    // Lowest byte of IP: lower bound (1) and upper bound (253)
    // 3232236801 = 192.168.5.1
    EXPECT_NO_THROW(NetworkAddress(3232236801, 8080));
    // 3232237053 = 192.168.5.253
    EXPECT_NO_THROW(NetworkAddress(3232237053, 8080)); 
}

TEST(NetworkAddressTest, IntConstructor_InvalidPort)
{
    EXPECT_THROW(NetworkAddress(3232236801, 1023), std::invalid_argument);
    EXPECT_THROW(NetworkAddress(3232236801, 80), std::invalid_argument);
    EXPECT_THROW(NetworkAddress(3232236801, 0), std::invalid_argument);
    
    EXPECT_THROW(NetworkAddress(3232236801, 65536), std::invalid_argument);
    EXPECT_THROW(NetworkAddress(3232236801, 65537), std::invalid_argument);
    
    EXPECT_THROW(NetworkAddress(3232236801, -1), std::invalid_argument);
}

TEST(NetworkAddressTest, IntConstructor_InvalidIP)
{
    // The lowest octet == 0 (192.168.5.0 = 3232236800)
    EXPECT_THROW(NetworkAddress(3232236800, 8080), std::invalid_argument);
    
    // The lowest octet >= 254 (192.168.5.254 = 3232237054)
    EXPECT_THROW(NetworkAddress(3232237054, 8080), std::invalid_argument);
    EXPECT_THROW(NetworkAddress(3232237055, 8080), std::invalid_argument); // 255

    // The entire IP is less than 0 or more than 32-bit
    EXPECT_THROW(NetworkAddress(-1, 8080), std::invalid_argument);
    EXPECT_THROW(NetworkAddress(4294967296, 8080), std::invalid_argument); // 0x100000000
}

// Tests for NetworkAddress(const std::string& ipAddressStr, const std::string& portStr)
TEST(NetworkAddressTest, StringPairConstructor_Valid)
{
    NetworkAddress addr("0127.0.0.2", "5432");
    
    EXPECT_EQ(addr.getPort(), 5432);
    EXPECT_EQ(addr.getIP().octets[0], 127);
    EXPECT_EQ(addr.getIP().octets[1], 0);
    EXPECT_EQ(addr.getIP().octets[2], 0);
    EXPECT_EQ(addr.getIP().octets[3], 2);
}

TEST(NetworkAddressTest, StringPairConstructor_ValidEdgeCases)
{
    EXPECT_NO_THROW(NetworkAddress("0.0.0.1", "1024"));
    EXPECT_NO_THROW(NetworkAddress("255.255.255.253", "1024"));

    // Port: lower bound (1024) and upper bound (65535)
    EXPECT_NO_THROW(NetworkAddress("192.168.1.32", "1024"));
    EXPECT_NO_THROW(NetworkAddress("192.168.1.32", "65535"));

    // Lowest byte of IP: lower bound (1) and upper bound (253)
    EXPECT_NO_THROW(NetworkAddress("192.168.1.1", "65535"));
    EXPECT_NO_THROW(NetworkAddress("192.168.1.253", "65535")); 
}

TEST(NetworkAddressTest, StringPairConstructor_InvalidPortStr)
{
    EXPECT_THROW(NetworkAddress("192.168.1.32", "abc"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32", "8080a"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32", ""), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32", "80l80"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32", "-7809"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32", "-1"), std::invalid_argument);

    EXPECT_THROW(NetworkAddress("192.168.1.32", "65536"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32", "65537"), std::invalid_argument);

    EXPECT_THROW(NetworkAddress("192.168.1.32", "1023"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32", "80"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32", "0"), std::invalid_argument);
}

TEST(NetworkAddressTest, StringPairConstructor_InvalidIPStr)
{
    // Invalid IP format
    EXPECT_THROW(NetworkAddress("192.168.32", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.32.1.1", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.a.32", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.32.c", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.d.32.32", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("k.168.1.32", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192..1.32", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.l4.1.32", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("-192.3.1.32", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.34.1.32b", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.255.1.-1", "8080"), std::invalid_argument);

    // The numbers go over the octet range (> 255)
    EXPECT_THROW(NetworkAddress("256.168.1.1", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.256.1.1", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.3.256.1", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.255.1.256", "8080"), std::invalid_argument);
    
    // The lowest octet is not valid (0 or >= 254)
    EXPECT_THROW(NetworkAddress("192.168.1.0", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.254", "8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.255", "8080"), std::invalid_argument);
}

// Tests for NetworkAddress(const std::string& fullAddressStr)
TEST(NetworkAddressTest, FullStringConstructor_Valid)
{
    NetworkAddress addr("172.16.254.10:49152");

    EXPECT_EQ(addr.getPort(), 49152);
    EXPECT_EQ(addr.getIP().octets[0], 172);
    EXPECT_EQ(addr.getIP().octets[1], 16);
    EXPECT_EQ(addr.getIP().octets[2], 254);
    EXPECT_EQ(addr.getIP().octets[3], 10);
}

TEST(NetworkAddressTest, FullStringConstructor_ValidEdgeCases)
{
    EXPECT_NO_THROW(NetworkAddress("0.0.0.1:1024"));
    EXPECT_NO_THROW(NetworkAddress("255.255.255.253:1024"));

    // Port: lower bound (1024) and upper bound (65535)
    EXPECT_NO_THROW(NetworkAddress("192.168.1.32:1024"));
    EXPECT_NO_THROW(NetworkAddress("192.168.1.32:65535"));

    // Lowest byte of IP: lower bound (1) and upper bound (253)
    EXPECT_NO_THROW(NetworkAddress("192.168.1.1:65535"));
    EXPECT_NO_THROW(NetworkAddress("192.168.1.253:65535")); 
}

TEST(NetworkAddressTest, FullStringConstructor_InvalidFormat)
{
    // There is no ':'
    EXPECT_THROW(NetworkAddress("192.168.1.1"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.1;8080"), std::invalid_argument);
    
    // Several ':'
    EXPECT_THROW(NetworkAddress("192.168.1.1:8080:123"), std::invalid_argument);

    // Empty parts
    EXPECT_THROW(NetworkAddress(":8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.1:"), std::invalid_argument);

    EXPECT_THROW(NetworkAddress("192.168.1.1.1:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192..1.32", "8080"), std::invalid_argument);
}

TEST(NetworkAddressTest, FullStringConstructor_InvalidPort)
{
    EXPECT_THROW(NetworkAddress("192.168.1.32:abc"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32:8080a"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32:80l80"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32:-7809"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32:-1"), std::invalid_argument);

    EXPECT_THROW(NetworkAddress("192.168.1.32:65536"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32:65537"), std::invalid_argument);

    EXPECT_THROW(NetworkAddress("192.168.1.32:1023"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32:80"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.32:0"), std::invalid_argument);
}

TEST(NetworkAddressTest, FullStringConstructor_InvalidIP)
{
    EXPECT_THROW(NetworkAddress("192.168.a.32:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.32.c:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.d.32.32:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("k.168.1.32:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.l4.1.32:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("-192.3.1.32:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.34.1.32b:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.255.1.-1:8080"), std::invalid_argument);

    // The numbers go over the octet range (> 255)
    EXPECT_THROW(NetworkAddress("256.168.1.1:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.256.1.1:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.3.256.1:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.255.1.256:8080"), std::invalid_argument);
    
    // The lowest octet is not valid (0 or >= 254)
    EXPECT_THROW(NetworkAddress("192.168.1.0:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.254:8080"), std::invalid_argument);
    EXPECT_THROW(NetworkAddress("192.168.1.255:8080"), std::invalid_argument);
}