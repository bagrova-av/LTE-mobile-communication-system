#pragma once
#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <optional>

#include "types.h"

class ClientSocket
{
private:
    int socketFileDescriptor;

public:
    ClientSocket();
    ~ClientSocket();

    ClientSocket(const ClientSocket& other) = delete;
    ClientSocket& operator=(const ClientSocket& other) = delete;

    bool connectToServer(const IPv4& ip, Port port);
    bool sendAllData(const BytesBuffer& data) const;
    bool sendByte(uint8_t byte) const;
    std::optional<BytesBuffer> receiveAllData(size_t bytesCount) const;
    void disconnect();
};

#endif