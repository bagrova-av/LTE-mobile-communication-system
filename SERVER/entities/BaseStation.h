#pragma once
#ifndef BASESTATION_H
#define BASESTATION_H

#include <unordered_map>
#include <mutex>

#include "ClientContext.h"
#include "../../CORE/entitiesConfigs/BaseStationConfig.h"

class BaseStation
{
private:
    BaseStationConfig config;
    std::unordered_map<M_TMSI, std::unique_ptr<ClientContext>> connectedClients;
    std::mutex bsMutex;

    ClientContext* findClientContextWithoutMutex(M_TMSI m_tmsi);
    bool hasEnoughSpaceInBufferWithoutMutex(ClientContext* clientContext,
                                            const NetworkDataPacket& data) const;
    size_t getBufferPacketSize(const NetworkDataPacket& packet) const;

public:
    BaseStation(const BaseStationConfig& config);

    bool registerClient(M_TMSI m_tmsi, int clientFileDescriptor);
    void removeClient(M_TMSI m_tmsi);
    float calculateSignalStrength(const LocationPoint3D& clientLocation) const;
    bool pushMessageToBuffer(M_TMSI m_tmsi, const NetworkDataPacket& data);
    std::optional<NetworkDataPacket> popMessageFromBuffer(M_TMSI m_tmsi);

    std::unique_ptr<ClientContext> releaseClientContext(M_TMSI m_tmsi);
    void acceptHandoverContext(std::unique_ptr<ClientContext> clientContext);

    void setMME(MmeId otherMmeId);

    BaseStationId getId() const;
    LocationPoint3D getPosition() const;
    CoordinatesType getCoverageRadius() const;
    MmeId getIdMME() const;
    size_t getNumberSupportedConnections() const;
    size_t getBufferSizePerConnection() const;
};

inline void BaseStation::setMME(MmeId otherMmeId)
{
    config.mmeId = otherMmeId;
}

inline BaseStationId BaseStation::getId() const
{
    return config.id;
}

inline LocationPoint3D BaseStation::getPosition() const
{
    return config.position;
}

inline CoordinatesType BaseStation::getCoverageRadius() const
{
    return config.coverageRadius;
}

inline MmeId BaseStation::getIdMME() const
{
    return config.mmeId;
}

inline size_t BaseStation::getNumberSupportedConnections() const
{
    return config.numberSupportedConnections;
}

inline size_t BaseStation::getBufferSizePerConnection() const
{
    return config.bufferBytesSizePerConnection;
}

#endif