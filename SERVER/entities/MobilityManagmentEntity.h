#pragma once
#ifndef MOBILITY_MANAGMENT_ENTITY_H
#define MOBILITY_MANAGMENT_ENTITY_H

#include "../../CORE/types.h"
#include "BaseStation.h"
#include "./hlrAndEir/JsonHlrEir.h"

struct clientsData
{
    IMEI imei;
    MSISDN msisdn;
    BaseStationId currentBaseSationId;
};

class MobilityManagmentEntity
{
private:
    MmeId id;
    std::vector<std::unique_ptr<BaseStation>> baseStations;
    std::unordered_map<M_TMSI, clientsData> visitorLocationRegister;
    std::unique_ptr<HlrEirInterface> homeLocationAndEquipmentIdentityRegister;
    
    std::unordered_map<MSISDN, M_TMSI> msisdnLookupTmsi;
    std::unordered_map<IMSI, M_TMSI> imsiLookupTmsi;
    std::unordered_map<M_TMSI, IMSI> tmsiLookupImsi;
    M_TMSI nextTMSI = 100;
    std::mutex mmeMutex;

    BaseStation* getBaseStationById(BaseStationId baseStationId);
    M_TMSI generateTMSI();
    std::optional<BaseStationId> findBestBaseStation(const LocationPoint3D& clientLocation);
    bool detachClientWithoutMutex(M_TMSI m_tmsi); 

public:
    explicit MobilityManagmentEntity(MmeId id,
                                    std::vector<std::unique_ptr<BaseStation>> baseStations,
                                    std::unique_ptr<HlrEirInterface> hlrEir);

    std::optional<M_TMSI> attachClient(const IMEI& imei, const IMSI& imsi,
                                       BaseStationId baseStationId, int clientFileDescriptor);
    bool detachClient(M_TMSI m_tmsi);
    void startHandover(M_TMSI m_tmsi, const LocationPoint3D& newClientLocation);
    void sendSMStoSMSC();
    std::optional<std::pair<M_TMSI, BaseStationId>> findRoutingInfoForSMS(const MSISDN& msisdn);
};

#endif