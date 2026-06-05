#include "AppProtocol.h"
#include "../COMMON/networkUtils.h"
#include "../COMMON/logging/Logger.h"

#include <cstring>

bool AppProtocol::sendLocationRequest(const ClientSocket& clientSocket,
                                      uint8_t protocolHeader, 
                                      const BytesBuffer& data)
{
    if (!clientSocket.sendByte(protocolHeader))
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

std::optional<CoordinatesType> AppProtocol::receiveDistanceResponse(const ClientSocket& clientSocket)
{
    std::optional<BytesBuffer> responseDataNetworkFormat = clientSocket.receiveAllData(sizeof(CoordinatesType));
    if (!responseDataNetworkFormat.has_value())
    {
        Logger::warning("ClientSocket: failed to get a response from the server with distance");
        return std::nullopt;
    }

    NetworkBitsType_t<CoordinatesType> distanceNetworkFormat;
    std::memcpy(&distanceNetworkFormat, responseDataNetworkFormat.value().data(), sizeof(CoordinatesType));
    return convertNetworkToHostOrder<CoordinatesType>(distanceNetworkFormat);
}

std::optional<ReceivedDataPacket> AppProtocol::receiveLocationRequest(int clientFileDescriptor)
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

    std::optional<BytesBuffer> bufferForSize = ServerSocket::receiveAllData(clientFileDescriptor, sizeof(uint32_t));
    if (!bufferForSize.has_value())
    {
        return std::nullopt;
    }
    uint32_t bufferSizeNetworkFormat;
    std::memcpy(&bufferSizeNetworkFormat, bufferForSize.value().data(), sizeof(bufferSizeNetworkFormat));
    uint32_t bufferSizeHostFormat = convertNetworkToHostOrder<uint32_t>(bufferSizeNetworkFormat);

    std::optional<BytesBuffer> data = ServerSocket::receiveAllData(clientFileDescriptor, bufferSizeHostFormat);
    if (!data.has_value())
    {
        return std::nullopt;
    }

    return ReceivedDataPacket{header.value()[0], data.value()};
}

bool AppProtocol::sendDistanceResponse(int clientFileDescriptor, CoordinatesType distance)
{
    if (clientFileDescriptor == -1)
    {
        return false;
    }

    NetworkBitsType_t<CoordinatesType> distanceNetworkFormat = convertHostToNetworkOrder(distance);
    BytesBuffer bufferDistance(sizeof(distanceNetworkFormat));
    std::memcpy(bufferDistance.data(), &distanceNetworkFormat, sizeof(distanceNetworkFormat));

    return ServerSocket::sendAllData(clientFileDescriptor, bufferDistance);
}