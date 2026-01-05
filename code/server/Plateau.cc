#include "Plateau.h"

Plateau::Plateau(unsigned int w, unsigned int h) : width(w), height(h), grid({w, h})
{
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            if (y == 0 || y == height - 1 || x == 0 || x == width - 1)
            {
                grid({x, y}) = Case(CellType::Wall);
            }
            else if (y == height / 2 && x == width / 2)
            {
                grid({x, y}) = Case(CellType::Hut);
            }
            else
            {
                grid({x, y}) = Case(CellType::Floor);
            }
        }
    }
}

Case &Plateau::getCase(unsigned int x, unsigned int y)
{
    return grid({x, y});
}

const Case &Plateau::getCase(unsigned int x, unsigned int y) const
{
    return grid({x, y});
}

bool Plateau::isInside(unsigned int x, unsigned int y) const
{
    return x >= 0 && y >= 0 && x < width && y < height;
}

bool Plateau::isWalkable(unsigned int x, unsigned int y) const
{
    if (x < 0 || y < 0 || x >= width || y >= height)
        return false;

    CellType type = grid({x, y}).getType();
    return type == CellType::Floor || type == CellType::Hut;
}

bool Plateau::isOccupied(unsigned int x, unsigned int y, uint32_t excludeId, const std::vector<Player> &players) const
{
    for (auto &p : players)
    {
        if (p.id != excludeId && p.x == x && p.y == y)
            return true;
    }
    return false;
}

/******PRINT*****/
void Plateau::print() const
{
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            switch (grid({x, y}).getType())
            {
            case CellType::Floor:
                std::cout << ".";
                break;
            case CellType::Wall:
                std::cout << "#";
                break;
            case CellType::Hut:
                std::cout << "H";
                break;
            }
        }
        std::cout << "\n";
    }
}

void Plateau::printWithPlayers(const std::vector<Player> &players) const
{
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {

            bool playerHere = false;

            for (const auto &p : players)
            {
                if (p.x == x && p.y == y)
                {
                    std::cout << p.id;
                    playerHere = true;
                    break;
                }
            }

            if (!playerHere)
            {
                switch (grid({x, y}).getType())
                {
                case CellType::Floor:
                    std::cout << ".";
                    break;
                case CellType::Wall:
                    std::cout << "#";
                    break;
                case CellType::Hut:
                    std::cout << "H";
                    break;
                }
            }
        }
        std::cout << "\n";
    }
}

BoardCommon Plateau::toCommonData()
{
    BoardCommon bc = BoardCommon(width,height);
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            bc.grid({x, y}) = grid({x, y}).toCommonData();
        }
    }
    return bc;
}
