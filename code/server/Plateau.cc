#include "Plateau.h"


Plateau::Plateau(int w, int h): width(w), height(h), grid({static_cast<unsigned>(w), static_cast<unsigned>(h)})
{
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (y == 0 || y == height - 1 || x == 0 || x == width - 1) {
                grid({static_cast<unsigned>(x), static_cast<unsigned>(y)}) = Case(CellType::Wall);
            }
            else if (y == height / 2 && x == width / 2) {
                grid({static_cast<unsigned>(x), static_cast<unsigned>(y)}) = Case(CellType::Hut);
            }
            else {
                grid({static_cast<unsigned>(x), static_cast<unsigned>(y)}) = Case(CellType::Floor);
            }
        }
    }
}

Case& Plateau::getCase(int x, int y) {
    return grid({static_cast<unsigned>(x), static_cast<unsigned>(y)});
}

const Case& Plateau::getCase(int x, int y) const {
    return grid({static_cast<unsigned>(x), static_cast<unsigned>(y)});
}

bool Plateau::isInside(int x, int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

bool Plateau::isWalkable(int x, int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height)
        return false;

    CellType type = grid({static_cast<unsigned>(x), static_cast<unsigned>(y)}).getType();
    return type == CellType::Floor || type == CellType::Hut;
}


bool Plateau::isOccupied(int x, int y, uint32_t excludeId, const std::vector<Player>& players) const {
    for (auto& p : players) {
        if (p.id != excludeId && p.x == x && p.y == y) return true;
    }
    return false;
}


/******PRINT*****/
void Plateau::print() const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            switch (grid({static_cast<unsigned>(x), static_cast<unsigned>(y)}).getType()) {
                case CellType::Floor: std::cout << "."; break;
                case CellType::Wall:  std::cout << "#"; break;
                case CellType::Hut:   std::cout << "H"; break;
            }
        }
        std::cout << "\n";
    }
}


void Plateau::printWithPlayers(const std::vector<Player>& players) const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            bool playerHere = false;

            for (const auto& p : players) {
                if (p.x == x && p.y == y) {
                    std::cout << p.id;
                    playerHere = true;
                    break;
                }
            }

            if (!playerHere) {
                switch (grid({static_cast<unsigned>(x), static_cast<unsigned>(y)}).getType()) {
                    case CellType::Floor: std::cout << "."; break;
                    case CellType::Wall:  std::cout << "#"; break;
                    case CellType::Hut:   std::cout << "H"; break;
                }
            }
        }
        std::cout << "\n";
    }
}

BoardData Plateau::toData() const
{
    BoardData res;
    res.width = width;
    res.height = height;
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x) {
            res.grid({x, y}) = grid({x, y}).toData();
        }
    }
    return res;
}
