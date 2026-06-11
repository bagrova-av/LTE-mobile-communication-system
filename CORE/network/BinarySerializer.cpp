#include "BinarySerializer.h"
#include "../../COMMON/networkUtils.h"
#include "../../COMMON/logging/Logger.h"

#include <cstring>

// BytesBuffer BinarySerializer::serialize(const LocationPoint3D& point) const
// {
//     BytesBuffer dataNetworkFormat(sizeof(CoordinatesType) * LocationPoint3D::COUNT_COMPONENTS_IN_POINT);

//     auto networkX = convertHostToNetworkOrder(point.x);
//     auto networkY = convertHostToNetworkOrder(point.y);
//     auto networkZ = convertHostToNetworkOrder(point.z);

//     size_t offset = 0;
//     std::memcpy(dataNetworkFormat.data(), &networkX, sizeof(networkX));
//     offset += sizeof(networkX);
//     std::memcpy(dataNetworkFormat.data() + offset, &networkY, sizeof(networkY));
//     offset += sizeof(networkY);
//     std::memcpy(dataNetworkFormat.data() + offset, &networkZ, sizeof(networkZ));

//     return dataNetworkFormat;
// }

// std::optional<LocationPoint3D> BinarySerializer::deserialize(const BytesBuffer& dataNetworkFormat) const
// {
//     if (dataNetworkFormat.size() != sizeof(CoordinatesType) * LocationPoint3D::COUNT_COMPONENTS_IN_POINT)
//     {
//         Logger::warning("BinarySerializer warning: invalid data size received from network");
//         return std::nullopt;
//     }

//     NetworkBitsType_t<CoordinatesType> networkX, networkY, networkZ;
//     size_t offset = 0;
//     std::memcpy(&networkX, dataNetworkFormat.data(), sizeof(networkX));
//     offset += sizeof(networkX);
//     std::memcpy(&networkY, dataNetworkFormat.data() + offset, sizeof(networkY));
//     offset += sizeof(networkY);
//     std::memcpy(&networkZ, dataNetworkFormat.data() + offset, sizeof(networkZ));

//     return LocationPoint3D{ convertNetworkToHostOrder<CoordinatesType>(networkX),
//                             convertNetworkToHostOrder<CoordinatesType>(networkY),
//                             convertNetworkToHostOrder<CoordinatesType>(networkZ)
//     };
// }