#include "BinarySerializer.h"
#include "../../COMMON/networkUtils.h"
#include "../../COMMON/logging/Logger.h"

#include <cstring>

BytesBuffer BinarySerializer::serialize(const NetworkMessage& message) const
{
    size_t totalSize = 0;

    std::visit([&totalSize](const auto& arg)
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, AttachRequest>)
                    {
                        totalSize = sizeof(uint32_t) + arg.imsi.length()
                                    + sizeof(uint32_t) + arg.imei.length()
                                    + sizeof(uint32_t) + arg.msisdn.length()
                                    + sizeof(BaseStationId);
                    }
                    else if constexpr (std::is_same_v<T, SearchRequest>)
                    {
                        totalSize = sizeof(uint32_t) + arg.imsi.length()
                                + (sizeof(CoordinatesType) * LocationPoint3D::COUNT_COMPONENTS_IN_POINT);
                    }
                    else if constexpr (std::is_same_v<T, RrcResponse>)
                    {
                        totalSize = sizeof(BaseStationId);
                    }
                    else if constexpr (std::is_same_v<T, RrcReject>)
                    {
                        totalSize = sizeof(uint32_t) + arg.reason.length();
                    }
                    else if constexpr (std::is_same_v<T, AttachAccept>)
                    {
                        totalSize = sizeof(M_TMSI);
                    }
                    else if constexpr (std::is_same_v<T, AttachReject>)
                    {
                        totalSize = sizeof(uint32_t) + arg.reason.length();
                    }
                    else if constexpr (std::is_same_v<T, DetachRequest>)
                    {
                        totalSize = sizeof(M_TMSI);
                    }
                    else if constexpr (std::is_same_v<T, LocationPoint3D>)
                    {
                        totalSize = sizeof(CoordinatesType) * LocationPoint3D::COUNT_COMPONENTS_IN_POINT;
                    }
                    else if constexpr (std::is_same_v<T, DistanceResponse>)
                    {
                        totalSize = sizeof(CoordinatesType);
                    }
                    else if constexpr (std::is_same_v<T, SmsSendRequest>)
                    {
                        totalSize = sizeof(M_TMSI)
                                    + sizeof(uint32_t) + arg.receiverMsisdn.length()
                                    + sizeof(SmsId)
                                    + sizeof(uint32_t) + arg.text.length();
                    }
                    else if constexpr (std::is_same_v<T, SmsSubmitAccept>)
                    {
                        totalSize = sizeof(SmsId) + sizeof(SmsId);
                    }
                    else if constexpr (std::is_same_v<T, SmsIncomingMessage>)
                    {
                        totalSize = sizeof(M_TMSI)
                                    + sizeof(uint32_t) + arg.senderMsisdn.length()
                                    + sizeof(SmsId)
                                    + sizeof(uint32_t) + arg.text.length();
                    }
                    else if constexpr (std::is_same_v<T, SmsDeliveryAcknowledge>)
                    {
                        totalSize = sizeof(M_TMSI) + sizeof(SmsId) + sizeof(SmsDeliveryStatus);
                    }
                    else if constexpr (std::is_same_v<T, SmsStatusReport>)
                    {
                        totalSize = sizeof(M_TMSI) + sizeof(SmsId) + sizeof(SmsDeliveryStatus);
                    }
                    else if constexpr (std::is_same_v<T, ErrorResponse>)
                    {
                        totalSize = sizeof(uint32_t) + arg.message.length();
                    }
                }, message);

    BytesBuffer dataNetworkFormat(totalSize);
    size_t offset = 0;

    std::visit([&](const auto& arg)
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, AttachRequest>)
                    {
                        writeString(dataNetworkFormat, arg.imsi, offset);
                        writeString(dataNetworkFormat, arg.imei, offset);
                        writeString(dataNetworkFormat, arg.msisdn, offset);
                        writeValue(dataNetworkFormat, arg.baseStationId, offset);
                    }
                    else if constexpr (std::is_same_v<T, SearchRequest>)
                    {
                        writeString(dataNetworkFormat, arg.imsi, offset);
                        writeValue(dataNetworkFormat, arg.position.x, offset);
                        writeValue(dataNetworkFormat, arg.position.y, offset);
                        writeValue(dataNetworkFormat, arg.position.z, offset);
                    }
                    else if constexpr (std::is_same_v<T, RrcResponse>)
                    {
                        writeValue(dataNetworkFormat, arg.baseStationId, offset);
                    }
                    else if constexpr (std::is_same_v<T, RrcReject>)
                    {
                        writeString(dataNetworkFormat, arg.reason, offset);
                    }
                    else if constexpr (std::is_same_v<T, AttachAccept>)
                    {
                        writeValue(dataNetworkFormat, arg.m_tmsi, offset);
                    }
                    else if constexpr (std::is_same_v<T, AttachReject>)
                    {
                        writeString(dataNetworkFormat, arg.reason, offset);
                    }
                    else if constexpr (std::is_same_v<T, DetachRequest>)
                    {
                        writeValue(dataNetworkFormat, arg.m_tmsi, offset);
                    }
                    else if constexpr (std::is_same_v<T, LocationPoint3D>)
                    {
                        writeValue(dataNetworkFormat, arg.x, offset);
                        writeValue(dataNetworkFormat, arg.y, offset);
                        writeValue(dataNetworkFormat, arg.z, offset);
                    }
                    else if constexpr (std::is_same_v<T, DistanceResponse>)
                    {
                        writeValue(dataNetworkFormat, arg.distance, offset);
                    }
                    else if constexpr (std::is_same_v<T, SmsSendRequest>)
                    {
                        writeValue(dataNetworkFormat, arg.senderTmsi, offset);
                        writeString(dataNetworkFormat, arg.receiverMsisdn, offset);
                        writeValue(dataNetworkFormat, arg.smsId, offset);
                        writeString(dataNetworkFormat, arg.text, offset);
                    }
                    else if constexpr (std::is_same_v<T, SmsSubmitAccept>)
                    {
                        writeValue(dataNetworkFormat, arg.tempId, offset);
                        writeValue(dataNetworkFormat, arg.globalId, offset);
                    }
                    else if constexpr (std::is_same_v<T, SmsIncomingMessage>)
                    {
                        writeValue(dataNetworkFormat, arg.receiverTmsi, offset);
                        writeString(dataNetworkFormat, arg.senderMsisdn, offset);
                        writeValue(dataNetworkFormat, arg.smsId, offset);
                        writeString(dataNetworkFormat, arg.text, offset);
                    }
                    else if constexpr (std::is_same_v<T, SmsDeliveryAcknowledge>)
                    {
                        writeValue(dataNetworkFormat, arg.receiverTmsi, offset);
                        writeValue(dataNetworkFormat, arg.smsId, offset);
                        writeValue(dataNetworkFormat, arg.status, offset);
                    }
                    else if constexpr (std::is_same_v<T, SmsStatusReport>)
                    {
                        writeValue(dataNetworkFormat, arg.senderTmsi, offset);
                        writeValue(dataNetworkFormat, arg.smsId, offset);
                        writeValue(dataNetworkFormat, arg.status, offset);
                    }
                    else if constexpr (std::is_same_v<T, ErrorResponse>)
                    {
                        writeString(dataNetworkFormat, arg.message, offset);
                    }
                }, message);

    return dataNetworkFormat;
}

std::optional<NetworkMessage> BinarySerializer::deserialize(PacketType packetType,
                                                      const BytesBuffer& dataNetworkFormat) const
{
    size_t offset = 0;
    switch (packetType)
    {
        case PacketType::ATTACH_REQUEST :
        {
            std::optional<IMSI> imsi = readString(dataNetworkFormat, offset);
            if (!imsi.has_value())
            {
                return std::nullopt;
            }

            std::optional<IMEI> imei = readString(dataNetworkFormat, offset);
            if (!imei.has_value())
            {
                return std::nullopt;
            }

            std::optional<MSISDN> msisdn = readString(dataNetworkFormat, offset);
            if (!msisdn.has_value())
            {
                return std::nullopt;
            }

            std::optional<BaseStationId> baseStationId = readValue<BaseStationId>(dataNetworkFormat, offset);
            if (!baseStationId.has_value())
            {
                return std::nullopt;
            }

            return AttachRequest{ imsi.value(), imei.value(), msisdn.value(), baseStationId.value() };
        }
        case PacketType::RRC_SEARCH :
        {
            std::optional<IMSI> imsi = readString(dataNetworkFormat, offset);
            if (!imsi.has_value())
            {
                return std::nullopt;
            }

            std::optional<CoordinatesType> x, y, z;
            x = readValue<CoordinatesType>(dataNetworkFormat, offset);
            if (!x.has_value())
            {
                return std::nullopt;
            }
            y = readValue<CoordinatesType>(dataNetworkFormat, offset);
            if (!y.has_value())
            {
                return std::nullopt;
            }
            z = readValue<CoordinatesType>(dataNetworkFormat, offset);
            if (!z.has_value())
            {
                return std::nullopt;
            }

            return SearchRequest{ imsi.value(), LocationPoint3D{x.value(), y.value(), z.value()} };
        }
        case PacketType::RRC_RESPONSE :
        {
            std::optional<BaseStationId> baseStationId = readValue<BaseStationId>(dataNetworkFormat, offset);
            if (!baseStationId.has_value())
            {
                return std::nullopt;
            }

            return RrcResponse{ baseStationId.value() };
        }
        case PacketType::RRC_REJECT :
        {
            std::optional<std::string> reason = readString(dataNetworkFormat, offset);
            if (!reason.has_value())
            {
                return std::nullopt;
            }

            return RrcReject{ reason.value() };
        }
        case PacketType::ATTACH_ACCEPT :
        {
            std::optional<M_TMSI> m_tmsi = readValue<M_TMSI>(dataNetworkFormat, offset);
            if (!m_tmsi.has_value())
            {
                return std::nullopt;
            }

            return AttachAccept{ m_tmsi.value() };
        }
        case PacketType::ATTACH_REJECT :
        {
            std::optional<std::string> reason = readString(dataNetworkFormat, offset);
            if (!reason.has_value())
            {
                return std::nullopt;
            }

            return AttachReject{ reason.value() };
        }
        case PacketType::DETACH_REQUEST :
        {
            std::optional<M_TMSI> m_tmsi = readValue<M_TMSI>(dataNetworkFormat, offset);
            if (!m_tmsi.has_value())
            {
                return std::nullopt;
            }

            return DetachRequest{ m_tmsi.value() };
        }
        case PacketType::LOCATION :
        {
            std::optional<CoordinatesType> x, y, z;
            x = readValue<CoordinatesType>(dataNetworkFormat, offset);
            if (!x.has_value())
            {
                return std::nullopt;
            }
            y = readValue<CoordinatesType>(dataNetworkFormat, offset);
            if (!y.has_value())
            {
                return std::nullopt;
            }
            z = readValue<CoordinatesType>(dataNetworkFormat, offset);
            if (!z.has_value())
            {
                return std::nullopt;
            }

            return LocationPoint3D{x.value(), y.value(), z.value()};
        }
        case PacketType::DISTANCE:
        {
            std::optional<CoordinatesType> distance = readValue<CoordinatesType>(dataNetworkFormat, offset);
            if (!distance.has_value())
            {
                return std::nullopt;
            }

            return DistanceResponse{ distance.value() };
        }
        case PacketType::SMS_SEND :
        {
            std::optional<M_TMSI> senderTmsi = readValue<M_TMSI>(dataNetworkFormat, offset);
            if (!senderTmsi.has_value())
            {
                return std::nullopt;
            }

            std::optional<MSISDN> receiverMsisdn = readString(dataNetworkFormat, offset);
            if (!receiverMsisdn.has_value())
            {
                return std::nullopt;
            }

            std::optional<SmsId> smsId = readValue<SmsId>(dataNetworkFormat, offset);
            if (!smsId.has_value())
            {
                return std::nullopt;
            }

            std::optional<std::string> text = readString(dataNetworkFormat, offset);
            if (!text.has_value())
            {
                return std::nullopt;
            }

            return SmsSendRequest{ senderTmsi.value(),
                                   receiverMsisdn.value(),
                                   smsId.value(),
                                   text.value() };
        }
        case PacketType::SMS_SUBMIT_ACCEPT :
        {
            std::optional<SmsId> tempId = readValue<SmsId>(dataNetworkFormat, offset);
            if (!tempId.has_value())
            {
                return std::nullopt;
            }

            std::optional<SmsId> globalId = readValue<SmsId>(dataNetworkFormat, offset);
            if (!globalId.has_value())
            {
                return std::nullopt;
            }

            return SmsSubmitAccept{ tempId.value(), globalId.value() };
        }
        case PacketType::SMS_RECEIVE :
        {
            std::optional<M_TMSI> receiverTmsi = readValue<M_TMSI>(dataNetworkFormat, offset);
            if (!receiverTmsi.has_value())
            {
                return std::nullopt;
            }

            std::optional<MSISDN> senderMsisdn = readString(dataNetworkFormat, offset);
            if (!senderMsisdn.has_value())
            {
                return std::nullopt;
            }

            std::optional<SmsId> smsId = readValue<SmsId>(dataNetworkFormat, offset);
            if (!smsId.has_value())
            {
                return std::nullopt;
            }

            std::optional<std::string> text = readString(dataNetworkFormat, offset);
            if (!text.has_value())
            {
                return std::nullopt;
            }

            return SmsIncomingMessage{ receiverTmsi.value(),
                                       senderMsisdn.value(),
                                       smsId.value(),
                                       text.value() };
        }
        case PacketType::SMS_DELIVERY_ACKNOWLEDGE :
        {
            std::optional<M_TMSI> receiverTmsi = readValue<M_TMSI>(dataNetworkFormat, offset);
            if (!receiverTmsi.has_value())
            {
                return std::nullopt;
            }

            std::optional<SmsId> smsId = readValue<SmsId>(dataNetworkFormat, offset);
            if (!smsId.has_value())
            {
                return std::nullopt;
            }

            std::optional<uint8_t> status = readValue<uint8_t>(dataNetworkFormat, offset);
            if (!status.has_value())
            {
                return std::nullopt;
            }

            return SmsDeliveryAcknowledge{ receiverTmsi.value(),
                                            smsId.value(),
                                            static_cast<SmsDeliveryStatus>(status.value()) };
        }
        case PacketType::SMS_STATUS :
        {
            std::optional<M_TMSI> senderTmsi = readValue<M_TMSI>(dataNetworkFormat, offset);
            if (!senderTmsi.has_value())
            {
                return std::nullopt;
            }

            std::optional<SmsId> smsId = readValue<SmsId>(dataNetworkFormat, offset);
            if (!smsId.has_value())
            {
                return std::nullopt;
            }

            std::optional<uint8_t> status = readValue<uint8_t>(dataNetworkFormat, offset);
            if (!status.has_value())
            {
                return std::nullopt;
            }

            return SmsStatusReport{ senderTmsi.value(),
                                    smsId.value(),
                                    static_cast<SmsDeliveryStatus>(status.value()) };
        }
        case PacketType::ERROR_MESSAGE :
        {
            std::optional<std::string> message = readString(dataNetworkFormat, offset);
            if (!message.has_value())
            {
                return std::nullopt;
            }

            return ErrorResponse{ message.value() };
        }
        default :
        {
            return std::nullopt;
        }
    }
}

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