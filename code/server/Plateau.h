#pragma once

#include <gf/Array2D.h>
#include <gf/Vector.h>
#include <vector>
#include <cstdint>

#include "Case.h"
#include "ClientInfo.h"
#include "Player.h"


class Plateau {
public:
    Plateau(int width, int height);

    Case& getCase(int x, int y);
    const Case& getCase(int x, int y) const;

    bool isWalkable(int x, int y) const;
    bool isOccupied(int x, int y, uint32_t excludeId,
                    const std::vector<Player>& players) const;
    bool isInside(int x, int y) const;


    void print() const;
    void printWithPlayers(const std::vector<Player>& players) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    int width;
    int height;
    gf::Array2D<Case> grid;
};
