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

struct CaseCommon {
    static constexpr gf::Id type = "CaseCommon"_id;
    CellType celltype;
    bool pacGomme = false;
    CaseCommon() : celltype(CellType::Floor) {}
    CaseCommon(CellType t, bool pg) {celltype = t; pacGomme = pg;}
};
template<typename Archive>
  Archive& operator|(Archive& ar, CaseCommon& data) {
    return ar | data.celltype | data.pacGomme;
}

struct BoardCommon {
    BoardCommon() {};
    BoardCommon(unsigned int w, unsigned int h) : grid({w, h}), width(w), height(h) {}
    static constexpr gf::Id type = "BoardCommon"_id;
    unsigned int width;
    unsigned int height;
    gf::Array2D<CaseCommon> grid;
};
template<typename Archive>
  Archive& operator|(Archive& ar, BoardCommon& data) {
    return ar | data.width | data.height | data.grid;
}


struct GameState {
    static constexpr gf::Id type = "GameState"_id;
    std::vector<ClientState> clientStates;
    BoardCommon board;
};
template<typename Archive>
  Archive& operator|(Archive& ar, GameState& data) {
    return ar | data.clientStates | data.board;
}


