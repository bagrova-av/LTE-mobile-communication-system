#include "BaseStation.h"

BaseStation::BaseStation(const BaseStationConfig& config) :
    config(config)
{ }

bool BaseStation::registerClient(M_TMSI m_tmsi, int clientFileDescriptor)
{
    std::lock_guard<std::mutex> lock(bsMutex);

    if (connectedClients.size() >= config.numberSupportedConnections)
    {
        return false;
    }
    connectedClients.emplace(m_tmsi, std::make_unique<ClientContext>(m_tmsi, 
                                                                    clientFileDescriptor,
                                                                    UEState::CONNECTED
                                                                )
    );
    return true;
}

ClientContext* BaseStation::findClientContextWithoutMutex(M_TMSI m_tmsi)
{
    auto it = connectedClients.find(m_tmsi);
    if (it == connectedClients.end())
    {
        return nullptr;
    }
    return (it->second).get();
}

void BaseStation::removeClient(M_TMSI m_tmsi)
{
    std::lock_guard<std::mutex> lock(bsMutex);
    connectedClients.erase(m_tmsi);
}

float BaseStation::calculateSignalStrength(const LocationPoint3D& clientLocation) const
{
    CoordinatesType distance = config.position.distanceTo(clientLocation);
    if (distance > config.coverageRadius)
    {
        return std::numeric_limits<float>::lowest();
    }
    return 1.0f - (distance / config.coverageRadius);
}

bool BaseStation::pushMessageToBuffer(M_TMSI m_tmsi, const NetworkDataPacket& data)
{
    std::lock_guard<std::mutex> lock(bsMutex);

    ClientContext* clientContext = findClientContextWithoutMutex(m_tmsi);
    if (!clientContext)
    {
        return false;
    }

    if (hasEnoughSpaceInBufferWithoutMutex(clientContext, data))
    {
        clientContext->messageBuffer.push(data);
        clientContext->currentBufferBytesSize += getBufferPacketSize(data);
        return true;
    }
    return false;
}

std::optional<NetworkDataPacket> BaseStation::popMessageFromBuffer(M_TMSI m_tmsi)
{
    std::lock_guard<std::mutex> lock(bsMutex);

    ClientContext* clientContext = findClientContextWithoutMutex(m_tmsi);
    if (!clientContext)
    {
        return std::nullopt;
    }

    auto data = clientContext->messageBuffer.tryPop();
    if (!data.has_value())
    {
        return std::nullopt;
    }
    clientContext->currentBufferBytesSize -= getBufferPacketSize(data.value());
    return data;
}

bool BaseStation::hasEnoughSpaceInBufferWithoutMutex(ClientContext* clientContext,
                                                    const NetworkDataPacket& data) const
{
    if (!clientContext)
    {
        return false;
    }
    return clientContext->currentBufferBytesSize + getBufferPacketSize(data)
           <= config.bufferBytesSizePerConnection;
}

size_t BaseStation::getBufferPacketSize(const NetworkDataPacket& packet) const
{
    return sizeof(packet.packetType) + packet.data.size();
}

std::unique_ptr<ClientContext> BaseStation::releaseClientContext(M_TMSI m_tmsi)
{
    std::lock_guard<std::mutex> lock(bsMutex);

    auto it = connectedClients.find(m_tmsi);
    if (it == connectedClients.end())
    {
        return nullptr;
    }
    
    auto clientContext = std::move(it->second);
    connectedClients.erase(it);
    return clientContext;
}

void BaseStation::acceptHandoverContext(std::unique_ptr<ClientContext> clientContext)
{
    if (!clientContext)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(bsMutex);
    connectedClients.emplace(clientContext->m_tmsi, std::move(clientContext));
}