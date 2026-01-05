#pragma once
#include <cstdint>
#include <vector>
#include <gf/Id.h>
#include <gf/SerializationOps.h>
#include <gf/Array2D.h>
#include "Types.h"

using namespace gf::literals;

struct ClientState {
    static constexpr gf::Id type = "ClientState"_id;
    uint32_t id;
    float x, y;
    uint32_t color; // RGBA
};

template<typename Archive>
  Archive& operator|(Archive& ar, ClientState& data) {
    return ar | data.id | data.x | data.y | data.color;
}

struct ClientMove {
    static constexpr gf::Id type = "ClientMove"_id;
    char moveDir;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ClientMove& data) {
    return ar | data.moveDir;
}


struct GameState {
    static constexpr gf::Id type = "GameState"_id;
    std::vector<ClientState> clientStates;
};
template<typename Archive>
  Archive& operator|(Archive& ar, GameState& data) {
    return ar | data.clientStates;
}