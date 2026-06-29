#pragma once
#ifndef SERVER_APP_H
#define SERVER_APP_H

#include <atomic>
#include <future>

#include "../CORE/types.h"
#include "../CORE/ServerSocket.h"
#include "./entities/MobilityManagmentEntity.h"
#include "SerializerInterface.h"
#include "../../CORE/entitiesConfigs/EpcConfig.h"
#include "../CORE/network/SerializerFactory.h"
#include "../COMMON/logging/Logger.h"

class ServerApp
{
private:
    EpcConfig epcConfig;
    size_t MAX_ACTIVE_CLIENT_CONNECTIONS = 100;
    const Port listeningPort;
    const LocationPoint3D location;
    std::atomic<bool> isRunning;
    std::atomic<bool> isNetworkActive;
    ServerSocket serverSocket;
    std::vector<std::future<void>> activeClientsFutures;
    std::unique_ptr<MobilityManagmentEntity> mme;
    std::atomic<std::chrono::system_clock::time_point> lastClientDisconnectedTime;
    std::atomic<size_t> activeClientsCount;

    void networkThreadWorker(std::promise<bool> startPromise);
    void processSingleClient(int clientFileDescriptor);

    void closeClientConnection(int clientFileDescriptor);
    void checkActiveClientsCount();

    template<typename T>
    bool sendResponse(int fd, Protocol protocol, PacketType packetType, const T& payload);

    bool sendRrcResponse(int clientFileDescriptor, Protocol protocol, BaseStationId baseStationId);
    bool sendRrcReject(int clientFileDescriptor, Protocol protocol, const std::string& reason);
    bool sendAttachAccept(int clientFileDescriptor, Protocol protocol, M_TMSI m_tmsi);
    bool sendAttachReject(int clientFileDescriptor, Protocol protocol, const std::string& reason);
    bool sendDistanceResponse(int clientFileDescriptor, Protocol protocol, CoordinatesType distance);
    bool sendErrorResponse(int clientFileDescriptor, Protocol protocol, const std::string& message);

    bool sendSmsSubmitAccept(int clientFileDescriptor, Protocol protocol,
                            SmsId tempSmsId, SmsId globalSmsId, uint8_t totalParts);
    bool sendSmsSubmitReject(int clientFileDescriptor, Protocol protocol, SmsId tempSmsId, const std::string& reason);

    void sendBufferedMessages(int clientFileDescriptor, const std::optional<M_TMSI>& currentTmsi);

    bool handleRrcSearch(int clientFileDescriptor, Protocol protocol,
                        const SerializerInterface& serializer, const BytesBuffer& data);
    bool handleAttachRequest(int clientFileDescriptor, Protocol protocol,
                              const SerializerInterface& serializer, const BytesBuffer& data,
                              std::optional<M_TMSI>& currentTmsi);
    bool handleLocationRequest(int clientFileDescriptor, Protocol protocol,
                               const SerializerInterface& serializer, const BytesBuffer& data,
                               std::optional<M_TMSI>& currentTmsi);
    bool handleDetachRequest(int clientFileDescriptor, Protocol protocol,
                              const SerializerInterface& serializer, const BytesBuffer& data,
                              std::optional<M_TMSI>& currentTmsi);
    bool handleSmsSendRequest(int clientFileDescriptor, Protocol protocol,
                              const SerializerInterface& serializer, const BytesBuffer& data);
    bool handleSmsDeliveryAknowledge(int clientFileDescriptor, Protocol protocol,
                                     const SerializerInterface& serializer, const BytesBuffer& data);

public:
    ServerApp(uint16_t port, const LocationPoint3D& location, const EpcConfig& epcConfig);
    
    void process();
    void stop();

    Port getPort() const;
    const LocationPoint3D& getLocation() const;
};

inline Port ServerApp::getPort() const
{
    return listeningPort;
}

inline const LocationPoint3D& ServerApp::getLocation() const
{
    return location;
}

inline void ServerApp::stop()
{
    isRunning = false;
}

template<typename T>
bool ServerApp::sendResponse(int clientFileDescriptor, Protocol protocol,
                            PacketType packetType, const T& payload)
{
    auto serializer = SerializerFactory::createSerializer(protocol);
    if (!serializer)
    {
        Logger::warning("Received unknown protocol header from client");
        return false;
    }

    BytesBuffer data = serializer->serialize(payload);

    return PacketTransporter::sendDataPacketResponse(clientFileDescriptor,
                                                    static_cast<uint8_t>(protocol),
                                                    packetType,
                                                    data);
}

#endif