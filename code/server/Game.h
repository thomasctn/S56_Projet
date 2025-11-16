#pragma once

#include <cstdint>
#include <vector>
#include "Plateau.h"

struct Player {
    uint32_t id;
    float x, y;
};

class Game {
public:
    Game(int width, int height);

    bool canMove(uint32_t playerId, float newX, float newY) const;
    void movePlayer(uint32_t playerId, float newX, float newY);

    Plateau& getPlateau() { return plateau; }
    const Plateau& getPlateau() const { return plateau; }

    Player& getPlayerInfo(uint32_t playerId);
    const std::vector<Player>& getPlayers() const { return players; }

    void addPlayer(uint32_t id, float x, float y);

private:
    Plateau plateau;
    std::vector<Player> players;
};
