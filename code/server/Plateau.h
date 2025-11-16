#pragma once
#include "Case.h"
#include <vector>
#include <cstdint>

class Plateau {
public:
    Plateau(int width, int height);

    Case& getCase(int x, int y);
    const Case& getCase(int x, int y) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    struct PlayerInfo {
        int x, y;
        uint32_t id;
    };

    void print() const;
    void printWithPlayers(const std::vector<PlayerInfo>& players) const;
    bool isWalkable(int x, int y) const;
    bool isOccupied(int x, int y, uint32_t excludeId, const std::vector<PlayerInfo>& players) const;

private:
    int width;
    int height;
    std::vector<std::vector<Case>> grid;
};
