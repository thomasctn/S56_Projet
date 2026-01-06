#pragma once
#include <gf/TcpSocket.h>
#include "../common/Protocol.h"
#include "../common/Types.h"

class Player {
public:
    Player(uint32_t id = 0, PlayerRole role = PlayerRole::Spectator)
    : id(id)
    , role(role)
    , x(0.0f)
    , y(0.0f)
    , color(0xFFFFFFFF)
    , score(0)
    {}

    uint32_t id;
    PlayerRole role;
    float x, y;
    uint32_t color;
    gf::TcpSocket socket;
    int score;

    PlayerRole getRole() const { return role; }
    void setRole(PlayerRole r) { role = r; }

    PlayerData getState() const {
        return { id, x, y, color, "moi", role, true };
    }

    void setState(const PlayerData& state) {
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
