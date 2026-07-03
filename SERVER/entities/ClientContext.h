#pragma once
#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

#include <unordered_map>

#include "../../CORE/types/types.h"
#include "../../COMMON/ThreadSafeQueue.h"
#include "../../CORE/PacketTransporter.h"

struct ClientContext
{
    M_TMSI m_tmsi;
    int socketFileDescriptor;
    ThreadSafeQueue<NetworkDataPacket> messageBuffer;
    size_t currentBufferBytesSize;

    ClientContext(M_TMSI m_tmsi, int socketFileDescriptor) :
        m_tmsi(m_tmsi),
        socketFileDescriptor(socketFileDescriptor),
        currentBufferBytesSize(0)
    {};
};

#endif