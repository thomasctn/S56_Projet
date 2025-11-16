#include "Plateau.h"
#include <iostream>


Plateau::Plateau(int w, int h) : width(w), height(h), grid(h, std::vector<Case>(w)) {
    // Génération statique du plateau
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (y == 0 || y == height-1 || x == 0 || x == width-1) {
                grid[y][x] = Case(CellType::Wall); // bords
            } else if (y == height/2 && x == width/2) {
                grid[y][x] = Case(CellType::Hut); // maison centrale
            } else {
                grid[y][x] = Case(CellType::Floor); // sol normal
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