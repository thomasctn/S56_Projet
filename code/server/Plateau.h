#pragma once

#include <gf/Array2D.h>
#include <gf/Log.h>
#include <gf/Vector.h>
#include <vector>
#include <cstdint>
#include <iostream>
#include "../common/Protocol.h"
#include "Case.h"
#include "Player.h"


class Plateau {
public:
    Plateau(unsigned int width, unsigned int height);

    Case& getCase(unsigned int x, unsigned int y);
    const Case& getCase(unsigned int x, unsigned int y) const;

    bool isWalkable(unsigned int x, unsigned int y) const;
    bool isOccupied(unsigned int x, unsigned int y, uint32_t excludeId,
                    const std::vector<Player>& players) const;
    bool isInside(unsigned int x, unsigned int y) const;


    void print() const;
    void printWithPlayers(const std::vector<Player>& players) const;

    unsigned int getWidth() const { return width; }
    unsigned int getHeight() const { return height; }
    BoardCommon toCommonData();

private:
    unsigned int width;
    unsigned int height;
    gf::Array2D<Case> grid;

};
