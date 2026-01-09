#pragma once

#include <cstdint>
#include <gf/TcpSocket.h>

#include "../common/Protocol.h"
#include "../common/Types.h"


class Controller;

class Player {
public:
    Player(uint32_t id = 0, PlayerRole role = PlayerRole::Spectator);
    PlayerRole getRole() const;
    void setRole(PlayerRole r);
    PlayerData getState() const;
    void setState(const PlayerData& state);

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    // Autorisation du move
    Player(Player&&) = default;
    Player& operator=(Player&&) = default;

    // --- Game ---
    // Retourne true si quelque chose a été mangé
    bool eat(bool isPacGomme, Player* otherPlayer = nullptr);
    bool ready;
public:
    uint32_t id;
    PlayerRole role;
    float x;
    float y;
    uint32_t color;
    gf::TcpSocket socket;
    int score;
    Controller* controller = nullptr;

};
