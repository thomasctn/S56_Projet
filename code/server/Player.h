#pragma once

#include <cstdint>
#include <gf/TcpSocket.h>

#include "../common/Protocol.h"
#include "../common/Types.h"

class Player {
public:
    Player(uint32_t id = 0, PlayerRole role = PlayerRole::Spectator);

    // Interdiction de la copie (socket non copiable)
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    // Autorisation du move
    Player(Player&&) = default;
    Player& operator=(Player&&) = default;

    // --- Accesseurs ---
    PlayerRole getRole() const;
    void setRole(PlayerRole r);

    ClientState getState() const;
    void setState(const ClientState& state);

    // --- Gameplay ---
    // Retourne true si quelque chose a été mangé
    bool eat(bool isPacGomme, Player* otherPlayer = nullptr);

public:
    uint32_t id;
    PlayerRole role;
    float x;
    float y;
    uint32_t color;
    gf::TcpSocket socket;
    int score;
};
