#pragma once
#include <cstdint>
#include <gf/Packet.h>

struct PacketContext {
    uint32_t senderId;
    gf::Packet packet;
};
