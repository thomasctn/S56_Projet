#pragma once
#include <gf/TcpSocket.h>
#include "../common/Protocol.h"

enum class PlayerRole { Ghost, PacMan };


class Player {
public:
    Player(uint32_t id = 0)
        : id(id), x(0), y(0), color(0xFFFFFFFF) {}
    uint32_t id;
    float x, y;
    uint32_t color;
    gf::TcpSocket socket;

    ClientState getState() const {
        return {id, x, y, color};
    }

    void setState(const ClientState& state) {
        x = state.x;
        y = state.y;
        color = state.color;
    }
};
