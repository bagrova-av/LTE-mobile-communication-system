#pragma once
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <optional>
#include <sys/socket.h>

#include "types.h"

class ServerSocket
{
private:
    int listenerSocketFileDescriptor;

public:
    ServerSocket();
    ~ServerSocket();

    ServerSocket(const ServerSocket& other) = delete;
    ServerSocket& operator=(const ServerSocket& other) = delete;

    bool startListening(Port port, int backlog = SOMAXCONN);
    void stopListening();
    int acceptClientConnection();
    static void closeClientConnection(int clientFileDescriptor);

    static bool sendAllData(int clientFileDescriptor, const BytesBuffer& data);
    static bool sendByte(int clientFileDescriptor, uint8_t byte);
    static std::optional<BytesBuffer> receiveAllData(int clientFileDescriptor, size_t bytesCount);
};

#endif