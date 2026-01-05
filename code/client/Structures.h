#pragma once
#include <vector>
#include <cstdint>
#include "../common/Types.h"


struct CaseRec{
    CellType type;
};

struct mapRec{
    int width;
    int height;
    std::vector<std::vector<CaseRec>> grid;

};



