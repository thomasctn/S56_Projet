#include "Plateau.h"
#include <iostream>

Plateau::Plateau(int w, int h) : width(w), height(h), grid(h, std::vector<Case>(w)) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (y == 0 || y == height-1 || x == 0 || x == width-1) {
                grid[y][x] = Case(CellType::Wall);
            } else if (y == height/2 && x == width/2) {
                grid[y][x] = Case(CellType::Hut);
            } else {
                grid[y][x] = Case(CellType::Floor);
            }
        }
    }
}

Case& Plateau::getCase(int x, int y) {
    return grid[y][x];
}

const Case& Plateau::getCase(int x, int y) const {
    return grid[y][x];
}

void Plateau::print() const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            switch (grid[y][x].getType()) {
                case CellType::Floor: std::cout << "."; break;
                case CellType::Wall:  std::cout << "#"; break;
                case CellType::Hut:   std::cout << "H"; break;
            }
        }
        std::cout << "\n";
    }
}

void Plateau::printWithPlayers(const std::vector<PlayerInfo>& players) const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            bool playerHere = false;
            for (auto& p : players) {
                if (p.x == x && p.y == y) {
                    std::cout << p.id;
                    playerHere = true;
                    break;
                }
            }
            if (!playerHere) {
                switch (grid[y][x].getType()) {
                    case CellType::Floor: std::cout << "."; break;
                    case CellType::Wall:  std::cout << "#"; break;
                    case CellType::Hut:   std::cout << "H"; break;
                }
            }
        }
        std::cout << "\n";
    }
}

bool Plateau::isWalkable(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height) return false;
    CellType type = grid[y][x].getType();
    return type == CellType::Floor || type == CellType::Hut;
}

bool Plateau::isOccupied(int x, int y, uint32_t excludeId, const std::vector<PlayerInfo>& players) const {
    for (auto& p : players) {
        if (p.id != excludeId && p.x == x && p.y == y) return true;
    }
    return false;
}
