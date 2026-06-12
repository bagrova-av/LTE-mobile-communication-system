#include "PacketTransporter.h"
#include "../COMMON/networkUtils.h"
#include "../COMMON/logging/Logger.h"

#include <cstring>

bool PacketTransporter::sendDataPacketRequest(const ClientSocket& clientSocket,
                                              uint8_t protocolHeader,
                                              PacketType packetType,
                                              const BytesBuffer& data)
{
    if (!clientSocket.sendByte(protocolHeader))
    {
        return false;
    }

    if (!clientSocket.sendByte(static_cast<uint8_t>(packetType)))
    {
        return false;
    }

    uint32_t dataSizeHostFormat = static_cast<uint32_t>(data.size());
    uint32_t dataSizeNetworkFormat = convertHostToNetworkOrder(dataSizeHostFormat);
    BytesBuffer bufferForSize(sizeof(dataSizeNetworkFormat));
    std::memcpy(bufferForSize.data(), &dataSizeNetworkFormat, sizeof(dataSizeNetworkFormat));
    if (!clientSocket.sendAllData(bufferForSize))
    {
        return false;
    }

    return clientSocket.sendAllData(data);
}

std::optional<ReceivedDataPacket> PacketTransporter::receiveDataPacketResponse(const ClientSocket& clientSocket)
{
    std::optional<BytesBuffer> header = clientSocket.receiveAllData(1);
    if (!header.has_value())
    {
        return std::nullopt;
    }

    std::optional<BytesBuffer> packetType = clientSocket.receiveAllData(1);
    if (!packetType.has_value())
    {
        return std::nullopt;
    }
    if (packetType.value()[0] >= static_cast<uint8_t>(PacketType::UNKNOWN))
    {
        Logger::warning("ClientSocket: received unknown packet type");
        return std::nullopt;
    }

    std::optional<BytesBuffer> bufferForSize = clientSocket.receiveAllData(sizeof(uint32_t));
    if (!bufferForSize.has_value())
    {
        return std::nullopt;
    }
    uint32_t bufferSizeNetworkFormat;
    std::memcpy(&bufferSizeNetworkFormat, bufferForSize.value().data(), sizeof(bufferSizeNetworkFormat));
    uint32_t bufferSizeHostFormat = convertNetworkToHostOrder<uint32_t>(bufferSizeNetworkFormat);

    std::optional<BytesBuffer> data = clientSocket.receiveAllData(bufferSizeHostFormat);
    if (!data.has_value())
    {
        return std::nullopt;
    }

    return ReceivedDataPacket{header.value()[0], static_cast<PacketType>(packetType.value()[0]), data.value()};
}

std::optional<ReceivedDataPacket> PacketTransporter::receiveDataPacketRequest(int clientFileDescriptor)
{
    if (clientFileDescriptor == -1)
    {
        return std::nullopt;
    }

    std::optional<BytesBuffer> header = ServerSocket::receiveAllData(clientFileDescriptor, 1);
    if (!header.has_value())
    {
        return std::nullopt;
    }

    std::optional<BytesBuffer> packetType = ServerSocket::receiveAllData(clientFileDescriptor, 1);
    if (!packetType.has_value())
    {
        Logger::error("ServerSocket: client disconnected unexpectedly after protocol header");
        return std::nullopt;
    }
    if (packetType.value()[0] >= static_cast<uint8_t>(PacketType::UNKNOWN))
    {
        Logger::warning("ServerSocket: received unknown packet type");
        return std::nullopt;
    }

    std::optional<BytesBuffer> bufferForSize = ServerSocket::receiveAllData(clientFileDescriptor, sizeof(uint32_t));
    if (!bufferForSize.has_value())
    {
        Logger::warning("ServerSocket: the client disconnected before data transfer completed");
        return std::nullopt;
    }
    uint32_t bufferSizeNetworkFormat;
    std::memcpy(&bufferSizeNetworkFormat, bufferForSize.value().data(), sizeof(bufferSizeNetworkFormat));
    uint32_t bufferSizeHostFormat = convertNetworkToHostOrder<uint32_t>(bufferSizeNetworkFormat);

    std::optional<BytesBuffer> data = ServerSocket::receiveAllData(clientFileDescriptor, bufferSizeHostFormat);
    if (!data.has_value())
    {
        Logger::warning("ServerSocket: the client disconnected before data transfer completed");
        return std::nullopt;
    }

    return ReceivedDataPacket{header.value()[0], static_cast<PacketType>(packetType.value()[0]), data.value()};
}

bool PacketTransporter::sendDataPacketResponse(int clientFileDescriptor,
                                               uint8_t protocolHeader,
                                               PacketType packetType,
                                               const BytesBuffer& data)
{
    if (clientFileDescriptor == -1)
    {
        return false;
    }

    if (!ServerSocket::sendByte(clientFileDescriptor, protocolHeader))
    {
        return false;
    }

    if (!ServerSocket::sendByte(clientFileDescriptor, static_cast<uint8_t>(packetType)))
    {
        return false;
    }

    uint32_t dataSizeHostFormat = static_cast<uint32_t>(data.size());
    uint32_t dataSizeNetworkFormat = convertHostToNetworkOrder(dataSizeHostFormat);
    BytesBuffer bufferForSize(sizeof(dataSizeNetworkFormat));
    std::memcpy(bufferForSize.data(), &dataSizeNetworkFormat, sizeof(dataSizeNetworkFormat));
    if (!ServerSocket::sendAllData(clientFileDescriptor, bufferForSize))
    {
        return false;
    }

    return ServerSocket::sendAllData(clientFileDescriptor, data);
}