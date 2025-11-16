#pragma once
#include <cstdint>
#include <vector>

struct ClientState {
    uint32_t id;
    float x, y;
    uint32_t color; // RGBA
};

// Pour sérialisation manuelle dans Packet
inline void serialize(const ClientState& cs, std::vector<uint8_t>& buffer) {
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&cs.id), reinterpret_cast<const uint8_t*>(&cs.id) + sizeof(cs.id));
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&cs.x), reinterpret_cast<const uint8_t*>(&cs.x) + sizeof(cs.x));
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&cs.y), reinterpret_cast<const uint8_t*>(&cs.y) + sizeof(cs.y));
    buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(&cs.color), reinterpret_cast<const uint8_t*>(&cs.color) + sizeof(cs.color));
}

inline ClientState deserializeClientState(const uint8_t* data, size_t& offset) {
    ClientState cs;
    cs.id = *reinterpret_cast<const uint32_t*>(data + offset); offset += sizeof(uint32_t);
    cs.x = *reinterpret_cast<const float*>(data + offset); offset += sizeof(float);
    cs.y = *reinterpret_cast<const float*>(data + offset); offset += sizeof(float);
    cs.color = *reinterpret_cast<const uint32_t*>(data + offset); offset += sizeof(uint32_t);
    return cs;
}