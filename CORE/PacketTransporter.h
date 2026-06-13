#pragma once
#ifndef PACKET_TRANSPORTER_H
#define PACKET_TRANSPORTER_H

#include "ClientSocket.h"
#include "ServerSocket.h"

enum class PacketType : uint8_t
{
    RRC_SEARCH,
    RRC_RESPONSE,
    RRC_REJECT,
    ATTACH_REQUEST,
    ATTACH_ACCEPT,
    ATTACH_REJECT,
    SMS_SEND,
    SMS_SUBMIT_ACCEPT,
    SMS_RECEIVE,
    SMS_DELIVERY_ACKNOWLEDGE,
    SMS_STATUS,
    DETACH_REQUEST,
    ERROR_MESSAGE,
    LOCATION, //legacy for compatibility
    DISTANCE, //legacy for compatibility
    UNKNOWN
};

struct ReceivedDataPacket
{
    uint8_t protocolHeader;
    PacketType packetType;
    BytesBuffer data;
};

class PacketTransporter
{
public:
    static bool sendDataPacketRequest(const ClientSocket& clientSocket, 
                                      uint8_t protocolHeader,
                                      PacketType packetType,
                                      const BytesBuffer& data);
    static std::optional<ReceivedDataPacket> receiveDataPacketResponse(const ClientSocket& clientSocket);

    static std::optional<ReceivedDataPacket> receiveDataPacketRequest(int clientFileDescriptor);
    static bool sendDataPacketResponse(int clientFileDescriptor,
                                       uint8_t protocolHeader,
                                       PacketType packetType,
                                       const BytesBuffer& data);
};

#endif