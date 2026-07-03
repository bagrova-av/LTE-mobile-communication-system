#pragma once
#ifndef MOBILITY_MANAGMENT_ENTITY_H
#define MOBILITY_MANAGMENT_ENTITY_H

#include "../../CORE/types/types.h"
#include "../../CORE/types/Protocol.h"
#include "BaseStation.h"
#include "./hlrAndEir/JsonHlrEir.h"
#include "../../CORE/entitiesConfigs/MmeConfig.h"
#include "SmsCenter.h"
#include "../../CORE/network/NetworkMessages.h"
#include "../../CORE/network/SerializerInterface.h"

struct ClientData
{
    IMEI imei;
    MSISDN msisdn;
    BaseStationId currentBaseSationId;
    Protocol protocol;
};

using SmsResult = std::variant<std::vector<SmsId>, std::string>;

class MobilityManagmentEntity
{
private:
    MmeConfig config;
    std::vector<std::unique_ptr<BaseStation>> baseStations;
    std::unordered_map<M_TMSI, ClientData> visitorLocationRegister;
    std::unique_ptr<HlrEirInterface> homeLocationAndEquipmentIdentityRegister;
    std::unique_ptr<SmsCenter> smsCenter;
    
    std::unordered_map<MSISDN, M_TMSI> msisdnLookupTmsi;
    std::unordered_map<IMSI, M_TMSI> imsiLookupTmsi;
    std::unordered_map<M_TMSI, IMSI> tmsiLookupImsi;
    M_TMSI nextTMSI = 100;
    std::mutex mmeMutex;

    M_TMSI generateTMSI();
    bool detachClientWithoutMutex(M_TMSI m_tmsi);
    std::optional<std::pair<M_TMSI, BaseStationId>> findRoutingInfoForSms(const MSISDN& receiverMsisdn);
    BaseStation* getBaseStationByIdWithoutMutex(BaseStationId baseStationId);
    BaseStation* getBaseStationByTmsiWithoutMutex(M_TMSI m_tmsi);

public:
    explicit MobilityManagmentEntity(const MmeConfig& config,
                                    std::vector<std::unique_ptr<BaseStation>> baseStations,
                                    std::unique_ptr<HlrEirInterface> hlrEir,
                                    std::unique_ptr<SmsCenter> smsCenter);

    BaseStation* getBaseStationById(BaseStationId baseStationId);
    BaseStation* getBaseStationByTmsi(M_TMSI m_tmsi);
    std::optional<BaseStationId> findBestBaseStation(const LocationPoint3D& clientLocation);

    void cleanupExpiredMessages();

    std::optional<M_TMSI> attachClient(const IMEI& imei, const IMSI& imsi,
                                       BaseStationId baseStationId, int clientFileDescriptor,
                                       Protocol protocol);
    bool detachClient(M_TMSI m_tmsi);
    void startHandover(M_TMSI m_tmsi, BaseStationId newBaseStationId);
    SmsResult handleSmsRequest(const SmsSendRequest& request);
    void handleSmsDeliveryAcknowledge(const SmsDeliveryAcknowledge& messages);
};

#endif