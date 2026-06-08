#include "BaseStation.h"

BaseStation::BaseStation(BaseStationId id, MmeId mmeId, const LocationPoint3D& position, CoordinatesType coverageRadius,
                        size_t numberSupportedConnections, size_t bufferBytesSizePerConnection) :
    id(id),
    mmeId(mmeId),
    position(position),
    coverageRadius(coverageRadius),
    numberSupportedConnections(numberSupportedConnections),
    bufferBytesSizePerConnection(bufferBytesSizePerConnection)
{} 

void BaseStation::registerClient(M_TMSI m_tmsi, int clientFileDescriptor)
{
    std::lock_guard<std::mutex> lock(bsMutex);

    if (connectedClients.size() >= numberSupportedConnections)
    {
        return;
    }
    connectedClients.emplace(m_tmsi, std::make_unique<ClientContext>(m_tmsi, 
                                                                    clientFileDescriptor,
                                                                    UEState::CONNECTED
                                                                )
    );
}

ClientContext* BaseStation::findClientContext(M_TMSI m_tmsi)
{
    auto it = connectedClients.find(m_tmsi);
    if (it == connectedClients.end())
    {
        return nullptr;
    }
    return (it->second).get();
}

float BaseStation::calculateSignalStrength(const LocationPoint3D& clientLocation) const
{
    CoordinatesType distance = position.distanceTo(clientLocation);
    if (distance > coverageRadius)
    {
        return std::numeric_limits<float>::lowest();
    }
    return 1.0f - (distance / coverageRadius); 
}

bool BaseStation::pushMessageToBuffer(M_TMSI m_tmsi, const BytesBuffer& data)
{
    std::lock_guard<std::mutex> lock(bsMutex);

    ClientContext* clientContext = findClientContext(m_tmsi);
    if (!clientContext)
    {
        return false;
    }

    if (hasEnoughSpaceInBuffer(clientContext, data))
    {
        clientContext->messageBuffer.push(data);
        clientContext->currentBufferBytesSize += data.size();
        return true;
    }
    return false;
}

std::optional<BytesBuffer> BaseStation::popMessageFromBuffer(M_TMSI m_tmsi)
{
    std::lock_guard<std::mutex> lock(bsMutex);

    ClientContext* clientContext = findClientContext(m_tmsi);
    if (!clientContext)
    {
        return std::nullopt;
    }

    auto data = clientContext->messageBuffer.tryPop();
    if (!data.has_value())
    {
        return std::nullopt;
    }
    clientContext->currentBufferBytesSize -= data.value().size();
    return data;
}

bool BaseStation::sendDeliveryStatusToСlient(M_TMSI m_tmsi)
{
    //TO-DO
    return true;
}

bool BaseStation::hasEnoughSpaceInBuffer(ClientContext* clientContext, const BytesBuffer& data) const
{
    if (!clientContext)
    {
        return false;
    }
    return clientContext->currentBufferBytesSize + data.size() <= bufferBytesSizePerConnection;
}