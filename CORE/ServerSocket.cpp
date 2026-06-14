#include "ServerSocket.h"
#include "../COMMON/logging/Logger.h"

#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <cmath>

ServerSocket::ServerSocket() :
    listenerSocketFileDescriptor(-1)
{ }

ServerSocket::~ServerSocket()
{
    stopListening();
}

bool ServerSocket::setAcceptTimeout(double seconds)
{
    struct timeval timeout;
    timeout.tv_sec = static_cast<long>(seconds);
    timeout.tv_usec = std::llround((seconds - timeout.tv_sec) * 1000000);
    if (setsockopt(listenerSocketFileDescriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        Logger::error("ServerSocket: failed when setting the timeout for input functions ("
                      + std::string(std::strerror(errno)) + ")");
        stopListening();
        return false;
    }
    return true;
}

void ServerSocket::stopListening()
{
    if (listenerSocketFileDescriptor != -1)
    {
        Logger::info("ServerSocket: listener stopped");
        shutdown(listenerSocketFileDescriptor, SHUT_RDWR);
        close(listenerSocketFileDescriptor);
        listenerSocketFileDescriptor = -1;
    }
}

bool ServerSocket::startListening(Port port, int backlog)
{
    listenerSocketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (listenerSocketFileDescriptor == -1)
    {
        Logger::error("ServerSocket: unable to create a socket ("
                      + std::string(std::strerror(errno)) + ")");
        return false;
    }
    Logger::debug("ServerSocket: created successfully (file descriptor = "
                    + std::to_string(listenerSocketFileDescriptor) + ")");

    int option = 1;
    if (setsockopt(listenerSocketFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        Logger::error("ServerSocket: failed to enable address reuse for the socket ("
                      + std::string(std::strerror(errno)) + ")");
        stopListening();
        return false;
    }

    sockaddr_in serverAddress;
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = htonl (INADDR_ANY);

    if (bind(listenerSocketFileDescriptor, 
             reinterpret_cast<struct sockaddr*>(&serverAddress),
             sizeof(serverAddress)) < 0)
    {
        Logger::error("ServerSocket: failed to bind to the port " + std::to_string(port)
                      + " (" + std::string(std::strerror(errno)) + ")");
        stopListening();
        return false;
    }
    Logger::info("ServerSocket: bound to the port " + std::to_string(port));

    if (listen(listenerSocketFileDescriptor, backlog) < 0)
    {
        Logger::error("ServerSocket: failed to listen on a socket ("
                      + std::string(std::strerror(errno)) + ")");
        stopListening();
        return false;
    }
    Logger::info("ServerSocket: listening for incoming connections");
    return true;
}

int ServerSocket::acceptClientConnection()
{
    if (listenerSocketFileDescriptor == -1)
    {
        return -1;
    }

    sockaddr_in clientAddress;
    socklen_t clientLen = sizeof(clientAddress);
    int clientFileDescriptor = accept(listenerSocketFileDescriptor,
                                      reinterpret_cast<struct sockaddr*>(&clientAddress),
                                      &clientLen);
    if (clientFileDescriptor < 0)
    {
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100'000; // 100 milliseconds
    if (setsockopt(clientFileDescriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        Logger::error("ServerSocket: failed when setting the timeout for input functions"
                      " (" + std::string(std::strerror(errno)) + ")");
        return -1;
    }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddress.sin_addr, clientIp, sizeof(clientIp));
    Logger::info("ServerSocket: the client connected from " + std::string(clientIp)
                 + ":" + std::to_string(ntohs(clientAddress.sin_port)));
    return clientFileDescriptor;
}

void ServerSocket::closeClientConnection(int clientFileDescriptor)
{
    if (clientFileDescriptor != -1)
    {
        close(clientFileDescriptor);
    }
}

bool ServerSocket::sendAllData(int clientFileDescriptor, const BytesBuffer& data)
{
    if (clientFileDescriptor == -1)
    {
        return false;
    }

    size_t totalSent = 0;
    size_t totalNeedSend = data.size();
    const uint8_t* ptrBuffer = data.data();
    while(totalSent < totalNeedSend)
    {
        ssize_t sent = send(clientFileDescriptor,
                           ptrBuffer + totalSent,
                           totalNeedSend - totalSent,
                           0);
        if (sent == -1)
        {
            Logger::error("ServerSocket: sending data to the client failed ("
                          + std::string(std::strerror(errno)) + ")");
            return false;
        }
        totalSent += sent;
    }
    return true;
}

bool ServerSocket::sendByte(int clientFileDescriptor, uint8_t byte)
{
    return sendAllData(clientFileDescriptor, BytesBuffer{byte});
}

std::optional<BytesBuffer> ServerSocket::receiveAllData(int clientFileDescriptor, size_t bytesCount)
{
    if (clientFileDescriptor == -1)
    {
        return std::nullopt;
    }

    BytesBuffer buffer(bytesCount);
    size_t totalReceived = 0;
    while (totalReceived < bytesCount)
    {
        ssize_t received = recv(clientFileDescriptor,
                                buffer.data() + totalReceived,
                                bytesCount - totalReceived,
                                0);
        if (received == 0)
        {
            //Logger::warning("ServerSocket: the client disconnected before data transfer completed");
            return std::nullopt;
        }
        if (received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) // recv() timed out
            {
                return std::nullopt;
            }
            Logger::error("ServerSocket: receiving data from the client failed ("
                          + std::string(std::strerror(errno)) + ")");
            return std::nullopt;
        }
        totalReceived += received;
    }
    return buffer;
}