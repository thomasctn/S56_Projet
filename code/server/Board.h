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
#include <random>
#include <unordered_map>
#include "../common/Types.h"





class Board {
public:
    Board(unsigned int width, unsigned int height);

    Case& getCase(unsigned int x, unsigned int y);
    const Case& getCase(unsigned int x, unsigned int y) const;

    bool isWalkable(unsigned int x, unsigned int y) const;
    bool isOccupied(unsigned int x, unsigned int y, uint32_t excludeId,
                    const std::vector<Player>& players) const;
    bool isInside(unsigned int x, unsigned int y) const;
    void placeRandomPacGommes(unsigned int count);


    void print() const;
    void printWithPlayers(const std::vector<Player>& players) const;

    unsigned int getWidth() const { return width; }
    unsigned int getHeight() const { return height; }

    std::unordered_map<Position, PacGommeType> getPacgommes() const {return pacgommes;}
    bool hasPacgomme(unsigned int x, unsigned int y) const;
    bool removePacgomme(unsigned int x, unsigned int y);
    PacGommeType takePacGomme(unsigned int x, unsigned int y);

    BoardCommon toCommonData();

    unsigned int getPacgommeCount() const {return pacgommes.size();}
    void openBorderExits(unsigned int count);
    bool isHole(unsigned int x, unsigned int y) const;
    std::vector<Position> getHoles() const;

    std::map<Position, Position> holeLinks;
        void linkHoles();
    Position getLinkedHole(unsigned int x, unsigned int y) const;

private:
    unsigned int width;
    unsigned int height;
    gf::Array2D<Case> grid;
    std::unordered_map<Position, PacGommeType> pacgommes;

    void generateTestMaze();
    void generateMaze();
    void generatePrimMaze();
    void placeHut();
    void connectHut();
    void openCorners();
    void addLoops(float probability);
    void fillDeadEnds();
    bool isHutWall(unsigned int x, unsigned int y);
};

