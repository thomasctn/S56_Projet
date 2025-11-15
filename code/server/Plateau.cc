#include "Plateau.h"

Plateau::Plateau(int w, int h) : width(w), height(h), grid(h, std::vector<Case>(w)) {
    // Génération statique du plateau
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (y == 0 || y == height-1 || x == 0 || x == width-1) {
                grid[y][x] = Case(CaseType::Wall); // bords du plateau
            } else if (y == height/2 && x == width/2) {
                grid[y][x] = Case(CaseType::Hut); // maison centrale
            } else {
                grid[y][x] = Case(CaseType::Floor); // sol normal
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
