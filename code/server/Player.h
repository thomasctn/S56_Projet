#pragma once
#include <gf/TcpSocket.h>
#include "../common/Protocol.h"

enum class PlayerRole {
    PacMan,
    Ghost,
    Spectator
};

class Player {
public:
    Player(uint32_t id = 0, PlayerRole role = PlayerRole::Spectator)
    : id(id)
    , role(role)
    , x(0.0f)
    , y(0.0f)
    , color(0xFFFFFFFF)
    {}

    uint32_t id;

    float x, y;
    uint32_t color;
    gf::TcpSocket socket;

    ClientState getState() const {
        return { id, x, y, color };
    }

    void setState(const ClientState& state) {
        x = state.x;
        y = state.y;
        color = state.color;
    }
};
