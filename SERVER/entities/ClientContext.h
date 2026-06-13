#pragma once
#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

#include <unordered_map>

#include "../../CORE/types.h"
#include "../../COMMON/ThreadSafeQueue.h"
#include "../../CORE/PacketTransporter.h"

enum class UEState
{
    CONNECTED,
    OFF
};

struct BufferedPacket
{
    PacketType packetType;
    BytesBuffer data;
};

struct ClientContext
{
    M_TMSI m_tmsi;
    int socketFileDescriptor;
    UEState state;
    ThreadSafeQueue<BufferedPacket> messageBuffer;
    size_t currentBufferBytesSize;

    ClientContext(M_TMSI m_tmsi, int socketFileDescriptor, UEState state) :
        m_tmsi(m_tmsi),
        socketFileDescriptor(socketFileDescriptor),
        state(state),
        currentBufferBytesSize(0)
    {};
};

#endif