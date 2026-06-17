#pragma once
#ifndef APP_H
#define APP_H

#include <memory>
#include <string>
#include <thread>
#include <map>

#include "../../CORE/types.h"
#include "AppParams.h"
#include "../../COMMON/ThreadSafeQueue.h"
#include "../../CORE/ClientSocket.h"
#include "../../CORE/network/SerializerInterface.h"
#include "NetworkTask.h"
#include "SmsStorage.h"
#include "../../CORE/entitiesConfigs/AddressBookEntry.h"

class App
{
private:
    const AppConfig config;
    AppState state;
    mutable std::mutex stateMutex;
    ClientSocket clientSocket;
    ThreadSafeQueue<NetworkTask> networkTasks;
    ThreadSafeQueue<std::string> responsesQueue;
    std::thread networkThread;
    SmsStorage smsStorage;
    std::atomic<SmsId> nextLocalSmsId;
    std::map<size_t, AddressBookEntry> addressBook;

    void networkThreadWorker();
    void handleAttachTask(uint8_t protocolHeader,
                          const SerializerInterface& serializer);
    void handleDetachTask(uint8_t protocolHeader,
                          const SerializerInterface& serializer);
    void handleLocationTask(uint8_t protocolHeader,
                            const SerializerInterface& serializer,
                            const NetworkTask& networkTask);
    void handleSendSmsTask(uint8_t protocolHeader,
                            const SerializerInterface& serializer,
                            const NetworkTask& networkTask);
    void handleIncomingServerMessage(const NetworkDataPacket& dataPacket);
    void pushTaskToQueue(TaskType type, Protocol protocol, TaskData data);

public:
    App(const AppConfig& appConfig, const AppState& initialState,
        const std::map<size_t, AddressBookEntry>& addressBook);
    ~App();

    void setStatus(bool status);
    void setLocation(const LocationPoint3D& newLocation, const LocationFlags& flags);
    void setProtocol(Protocol protocol);

    SmsId generateNextLocalSmsId();

    const IPv4& getIP() const;
    Port getPort() const;
    const NetworkAddress& getNetworkAddress() const;
    const IMEI& getIMEI() const;
    const IMSI& getIMSI() const;
    const MSISDN& getMSISDN() const;
    const Nodes& getNodes() const;
    const LocationPoint3D& getLocation() const;
    bool getStatus() const;
    Protocol getProtocol() const;
    std::optional<AddressBookEntry> getAddressBookEntryByShortcut(size_t shortcut) const;
    const std::map<size_t, AddressBookEntry>& getAddressBook() const;

    void addSentSmsRecord(const SentSmsRecord& smsRecord);
    void addReceivedSmsRecord(const ReceivedSmsRecord& smsRecord);
    std::vector<ReceivedSmsRecord> getReceivedSmsHistory() const;
    std::vector<SentSmsRecord> getSentSmsHistory() const;
    bool linkSmsIds(SmsId tempId, SmsId globalId, uint8_t totalParts);

    void sendLocationToServerAsync(const LocationPoint3D& location);
    void sendAttachTaskToServerAsync();
    void sendDetachTaskToServerAsync();
    void sendSmsTaskToServerAsync(const SmsTaskData& smsTaskData);
    bool processResponses();
};


inline void App::setStatus(bool status)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    state.isActive = status;
}

inline void App::setLocation(const LocationPoint3D& newLocation, const LocationFlags& flags)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    state.location.updateWithFlags(newLocation, flags);
}

inline void App::setProtocol(Protocol protocol)
{
    std::lock_guard<std::mutex> lock(stateMutex);
    state.protocol = protocol;
}

inline SmsId App::generateNextLocalSmsId()
{
    return nextLocalSmsId++;
}

inline const IPv4& App::getIP() const
{
    return config.netAddr.getIP(); 
}

inline Port App::getPort() const
{ 
    return config.netAddr.getPort(); 
}

inline const NetworkAddress& App::getNetworkAddress() const
{
    return config.netAddr;
}

inline const IMEI& App::getIMEI() const
{
    return config.imei;
}

inline const IMSI& App::getIMSI() const
{
    return config.imsi;
}

inline const MSISDN& App::getMSISDN() const
{
    return config.msisdn;
}

inline const Nodes& App::getNodes() const
{
    return config.nodes;
}

inline const LocationPoint3D& App::getLocation() const
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return state.location; 
}

inline bool App::getStatus() const
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return state.isActive;
}

inline Protocol App::getProtocol() const
{
    std::lock_guard<std::mutex> lock(stateMutex);
    return state.protocol;
}

inline const std::map<size_t, AddressBookEntry>& App::getAddressBook() const
{
    return addressBook;
}

inline void App::addSentSmsRecord(const SentSmsRecord& smsRecord)
{
    smsStorage.addSentSms(smsRecord);
}

inline void App::addReceivedSmsRecord(const ReceivedSmsRecord& smsRecord)
{
    smsStorage.addReceivedSms(smsRecord);
}

inline std::vector<ReceivedSmsRecord> App::getReceivedSmsHistory() const
{
    return smsStorage.getReceivedSmsCopy();
}

inline std::vector<SentSmsRecord> App::getSentSmsHistory() const
{
    return smsStorage.getSentSmsCopy();
}

inline bool App::linkSmsIds(SmsId tempId, SmsId globalId, uint8_t totalParts)
{
    return smsStorage.linkSmsIds(tempId, globalId, totalParts);
}

#endif