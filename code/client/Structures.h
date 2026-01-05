#pragma once
#include <vector>
#include <cstdint>



enum class CellType { Floor, Wall, Hut };

struct CaseRec{
    CellType type;
};

struct mapRec{
    int width;
    int height;
    std::vector<std::vector<CaseRec>> grid;

};



