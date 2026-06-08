#pragma once
#ifndef BASESTATION_H
#define BASESTATION_H

#include <unordered_map>
#include <mutex>

#include "ClientContext.h"

class BaseStation
{
private:
    BaseStationId id;
    LocationPoint3D position;
    CoordinatesType coverageRadius;
    MmeId mmeId;
    size_t numberSupportedConnections;
    size_t bufferBytesSizePerConnection;
    std::unordered_map<M_TMSI, std::unique_ptr<ClientContext>> connectedClients;
    std::mutex bsMutex;

    ClientContext* findClientContext(M_TMSI m_tmsi);
    bool hasEnoughSpaceInBuffer(ClientContext* clientContext, const BytesBuffer& data) const;

public:
    BaseStation(BaseStationId id, MmeId mmeId, const LocationPoint3D& position, CoordinatesType coverageRadius,
                size_t numberSupportedConnections, size_t bufferBytesSizePerConnection);

    void registerClient(M_TMSI m_tmsi, int clientFileDescriptor);
    float calculateSignalStrength(const LocationPoint3D& clientLocation) const;
    bool pushMessageToBuffer(M_TMSI m_tmsi, const BytesBuffer& data);
    std::optional<BytesBuffer> popMessageFromBuffer(M_TMSI m_tmsi);
    bool sendDeliveryStatusToСlient(M_TMSI m_tmsi);

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
    mmeId = otherMmeId;
}

inline BaseStationId BaseStation::getId() const
{
    return id;
}

inline LocationPoint3D BaseStation::getPosition() const
{
    return position;
}

inline CoordinatesType BaseStation::getCoverageRadius() const
{
    return coverageRadius;
}

inline MmeId BaseStation::getIdMME() const
{
    return mmeId;
}

inline size_t BaseStation::getNumberSupportedConnections() const
{
    return numberSupportedConnections;
}

inline size_t BaseStation::getBufferSizePerConnection() const
{
    return bufferBytesSizePerConnection;
}

#endif