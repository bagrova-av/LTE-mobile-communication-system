#include "ClientSocket.h"
#include "../COMMON/logging/Logger.h"
#include "../COMMON/networkUtils.h"
#include "socketUtils.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

ClientSocket::ClientSocket() :
    socketFileDescriptor(-1)
{ }

ClientSocket::~ClientSocket()
{
    disconnect();
}

void ClientSocket::disconnect()
{
    if (socketFileDescriptor != -1)
    {
        Logger::debug("ClientSocket: disconnected");
        close(socketFileDescriptor);
        socketFileDescriptor = -1;
    }
}

bool ClientSocket::connectToServer(const IPv4& ip, Port port)
{
    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor == -1)
    {
        Logger::error("ClientSocket: unable to create a socket ("
                      + std::string(std::strerror(errno)) + ")");
        return false;
    }
    Logger::debug("ClientSocket: created successfully (file descriptor = "
                    + std::to_string(socketFileDescriptor) + ")");

    sockaddr_in serverAddress;
    std::memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    std::memcpy(&serverAddress.sin_addr.s_addr, ip.octets.data(), 4);

    if (connect(socketFileDescriptor, 
                reinterpret_cast<struct sockaddr*>(&serverAddress), 
                sizeof(serverAddress)) < 0)
    {
        Logger::warning("ClientSocket: server is unavailable ("
                        + std::string(std::strerror(errno)) + ")");
        disconnect();
        return false;
    }

    if (!setSocketTimeout(socketFileDescriptor,
                          std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::milliseconds(100))))
    {
        Logger::error("ClientSocket: failed when setting the timeout for input functions"
                      " (" + std::string(std::strerror(errno)) + ")");
        disconnect();
        return false;
    }

    Logger::info("ClientSocket: connected to the server successfully");
    return true;
}

bool ClientSocket::sendAllData(const BytesBuffer& data) const
{
    if (socketFileDescriptor == -1)
    {
        return false;
    }

    size_t totalSent = 0;
    size_t totalNeedSend = data.size();
    const uint8_t* ptrBuffer = data.data();
    while(totalSent < totalNeedSend)
    {
        ssize_t sent = send(socketFileDescriptor, 
                            ptrBuffer + totalSent, 
                            totalNeedSend - totalSent,
                            0);
        if (sent == -1)
        {
            Logger::error("ClientSocket: sending data failed ("
                          + std::string(std::strerror(errno)) + ")");
            return false;
        }
        totalSent += sent;
    }
    return true;
}

bool ClientSocket::sendByte(uint8_t byte) const
{
    return sendAllData( BytesBuffer{byte} );
}

std::optional<BytesBuffer> ClientSocket::receiveAllData(size_t bytesCount) const
{
    if (socketFileDescriptor == -1)
    {
        return std::nullopt;
    }

    BytesBuffer buffer(bytesCount);
    size_t totalReceived = 0;
    while (totalReceived < bytesCount)
    {
        ssize_t received = recv(socketFileDescriptor,
                                buffer.data() + totalReceived,
                                bytesCount - totalReceived,
                                0);
        if (received == 0)
        {
            errno = 0;
            Logger::warning("ClientSocket: the server terminated the connection early");
            return std::nullopt;
        }
        if (received == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) // recv() timed out
            {
                return std::nullopt;
            }
            Logger::error("ClientSocket: receiving data failed ("
                          + std::string(std::strerror(errno)) + ")");
            return std::nullopt;
        }
        totalReceived += received;
    }
    return buffer;
}