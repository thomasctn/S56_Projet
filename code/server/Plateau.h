#pragma once

#include <gf/Array2D.h>
#include <gf/Vector.h>
#include <vector>
#include <cstdint>
#include <iostream>

#include "Case.h"
#include "Player.h"


class Plateau {
public:
    Plateau(size_t width, size_t height);

    Case& getCase(size_t x, size_t y);
    const Case& getCase(size_t x, size_t y) const;

    bool isWalkable(size_t x, size_t y) const;
    bool isOccupied(size_t x, size_t y, uint32_t excludeId,
                    const std::vector<Player>& players) const;
    bool isInside(size_t x, size_t y) const;


    void print() const;
    void printWithPlayers(const std::vector<Player>& players) const;

    size_t getWidth() const { return width; }
    size_t getHeight() const { return height; }
    BoardData toData() const;

private:
    size_t width;
    size_t height;
    gf::Array2D<Case> grid;

};
