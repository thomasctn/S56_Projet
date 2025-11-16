#pragma once
#include "Case.h"
#include <vector>

class Plateau {
public:
    Plateau(int width, int height);

    Case& getCase(int x, int y);
    const Case& getCase(int x, int y) const;

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void print() const;

private:
    int width;
    int height;
    std::vector<std::vector<Case>> grid;
};
