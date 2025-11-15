#pragma once
#include <gf/TcpSocket.h>
#include "../common/Types.h"

class Player {
public:
    Player(uint32_t id = 0) 
    : id(id), x(0), y(0), color(0xFFFFFFFF) {} // valeurs par défaut

    uint32_t id;
    float x, y;        // position
    uint32_t color;    // couleur du joueur
    gf::TcpSocket socket; // socket réseau (lié au client)

    // pour accéder à l'état global
    ClientState getState() const {
        return {id, x, y, color};
    }

    void setState(const ClientState& state) {
        x = state.x;
        y = state.y;
        color = state.color;
    }
};
