#pragma once
#include "Plateau.h"
#include <vector>
#include "Player.h"

class Game {
public:
    Game(int width, int height);

    void update(float dt);

    Plateau& getPlateau() { return plateau; }

private:
    Plateau plateau;
    std::vector<Player> players;
};
