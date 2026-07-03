#pragma once
#ifndef BASE_STATION_CONFIG_H
#define BASE_STATION_CONFIG_H

#include "../types/types.h"
#include "../types/Point3D.h"

struct BaseStationConfig
{
    BaseStationId id;
    MmeId mmeId;
    LocationPoint3D position;
    CoordinatesType coverageRadius;
    size_t numberSupportedConnections;
    size_t bufferBytesSizePerConnection;
};

#endif