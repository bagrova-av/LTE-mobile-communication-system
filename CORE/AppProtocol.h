#pragma once
#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

#include "ClientSocket.h"
#include "ServerSocket.h"

struct ReceivedDataPacket
{
    uint8_t protocolHeader;
    BytesBuffer data;
};

class AppProtocol
{
public:
    static bool sendLocationRequest(const ClientSocket& clientSocket, 
                                    uint8_t protocolHeader, 
                                    const BytesBuffer& data);
    static std::optional<CoordinatesType> receiveDistanceResponse(const ClientSocket& clientSocket);

    static std::optional<ReceivedDataPacket> receiveLocationRequest(int clientFileDescriptor);
    static bool sendDistanceResponse(int clientFileDescriptor, CoordinatesType distance);
};

#endif