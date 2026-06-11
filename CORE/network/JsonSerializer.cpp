#include "JsonSerializer.h"
#include "../../COMMON/logging/Logger.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

BytesBuffer JsonSerializer::serialize(const NetworkMessage& message) const
{
    json dataJSON;

    std::visit([&dataJSON](const auto& arg)
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, AttachRequest>)
                    {
                        dataJSON["imsi"] = arg.imsi;
                        dataJSON["imei"] = arg.imei;
                        dataJSON["msisdn"] = arg.msisdn;
                        dataJSON["baseStationId"] = arg.baseStationId;
                    }
                    else if constexpr (std::is_same_v<T, AttachAccept>)
                    {
                        dataJSON["m_tmsi"] = arg.m_tmsi;
                    }
                    else if constexpr (std::is_same_v<T, AttachReject>)
                    {
                        dataJSON["reason"] = arg.reason;
                    }
                    else if constexpr (std::is_same_v<T, SearchRequest>)
                    {
                        dataJSON["imsi"] = arg.imsi;
                        dataJSON["x"] = arg.position.x;
                        dataJSON["y"] = arg.position.y;
                        dataJSON["z"] = arg.position.z;
                    }
                    else if constexpr (std::is_same_v<T, RrcResponse>)
                    {
                        dataJSON["baseStationId"] = arg.baseStationId;
                    }
                    else if constexpr (std::is_same_v<T, RrcReject>)
                    {
                        dataJSON["reason"] = arg.reason;
                    }
                    else if constexpr (std::is_same_v<T, DetachRequest>)
                    {
                        dataJSON["m_tmsi"] = arg.m_tmsi;
                    }
                    else if constexpr (std::is_same_v<T, LocationPoint3D>)
                    {
                        dataJSON["x"] = arg.x;
                        dataJSON["y"] = arg.y;
                        dataJSON["z"] = arg.z;
                    }
                    else if constexpr (std::is_same_v<T, ErrorResponse>)
                    {
                        dataJSON["message"] = arg.message;
                    }
                }, message);

    std::string serializedData = dataJSON.dump();
    return BytesBuffer(serializedData.begin(), serializedData.end());
}

std::optional<NetworkMessage> JsonSerializer::deserialize(PacketType packetType,
                                                         const BytesBuffer& dataNetworkFormat) const
{
    json dataJSON = json::parse(std::string(dataNetworkFormat.begin(), dataNetworkFormat.end()));

    try
    {
        switch (packetType)
        {
            case PacketType::RRC_SEARCH :
            {
                 return SearchRequest{ dataJSON["imsi"],
                                       LocationPoint3D{dataJSON["x"], dataJSON["y"], dataJSON["z"]} };
            }
            case PacketType::RRC_RESPONSE :
            {
                return RrcResponse{ dataJSON["baseStationId"] };
            }
            case PacketType::RRC_REJECT :
            {
                return RrcReject{ dataJSON["reason"] };
            }
            case PacketType::ATTACH_REQUEST :
            {
                return AttachRequest{ dataJSON["imsi"],
                                      dataJSON["imei"],
                                      dataJSON["msisdn"],
                                      dataJSON["baseStationId"] };
            }
            case PacketType::ATTACH_ACCEPT :
            {
                return AttachAccept{ dataJSON["m_tmsi"] };
            }
            case PacketType::ATTACH_REJECT :
            {
                return AttachReject{ dataJSON["reason"] };
            }
            case PacketType::LOCATION :
            {
                return LocationPoint3D{ dataJSON["x"], dataJSON["y"], dataJSON["z"] };
            }
            case PacketType::DETACH_REQUEST :
            {
                return DetachRequest{ dataJSON["m_tmsi"] };
            }
            case PacketType::ERROR_MESSAGE :
            {
                return ErrorResponse{ dataJSON["message"] };
            }
            default :
            {
                return std::nullopt;
            }
        }
    }
    catch (const std::exception& ex)
    {
        Logger::error("JSON deserialize error: " + std::string(ex.what()));
        return std::nullopt;
    }
}

// BytesBuffer JsonSerializer::serialize(const LocationPoint3D& point) const
// {
//     json dataJSON;
//     dataJSON["x"] = point.x;
//     dataJSON["y"] = point.y;
//     dataJSON["z"] = point.z;

//     std::string serializedData = dataJSON.dump();
//     return BytesBuffer(serializedData.begin(), serializedData.end());
// }

// std::optional<LocationPoint3D> JsonSerializer::deserialize(const BytesBuffer& dataNetworkFormat) const
// {
//     json dataJSON = json::parse(std::string(dataNetworkFormat.begin(), dataNetworkFormat.end()));
    
//     return LocationPoint3D{ dataJSON["x"],
//                             dataJSON["y"],
//                             dataJSON["z"]
//     };
// }