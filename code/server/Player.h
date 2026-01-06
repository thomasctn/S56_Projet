#pragma once

#include <cstdint>
#include <gf/TcpSocket.h>
#include <gf/Log.h>

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
    bool eat(bool isPacGomme, Player* otherPlayer = nullptr);

public:
    uint32_t id;
    PlayerRole role;
    float x;
    float y;
    uint32_t color;
    gf::TcpSocket socket;
    int score;

    PlayerRole getRole() const { return role; }
    void setRole(PlayerRole r) { role = r; }

    ClientState getState() const {
        return { id, x, y, color };
    }

    void setState(const ClientState& state) {
        x = state.x;
        y = state.y;
        color = state.color;
    }

    // --- Fonction manger ---
    // Retourne vrai si quelque chose a été mangé
    bool eat(bool isPacGomme, Player* otherPlayer /* = nullptr */) {
        switch (role) {
            case PlayerRole::PacMan:
                if (isPacGomme) {
                    score += 10; // chaque pac-gomme = 10 points
                    gf::Log::info("PacMan %d a mangé une pac-gomme. Score=%d\n", id, score);
                    return true;
                }
                break;

            case PlayerRole::Ghost:
                if (otherPlayer && otherPlayer->getRole() == PlayerRole::PacMan) {
                    // respawn de PacMan
                    otherPlayer->x = 50.0f;
                    otherPlayer->y = 50.0f;
                    otherPlayer->score = 0;
                    gf::Log::info("Fantôme %d a mangé PacMan %d\n", id, otherPlayer->id);
                    return true;
                }
                break;

            default:
                break;
        }

        return false;
    }

};
