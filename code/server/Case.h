#pragma once
#include "../common/Types.h"

class Case {
public:
    Case() : type(CellType::Floor) {}
    Case(CellType t) : type(t) {}

    CellType getType() const { return type; }

    bool isWalkable() const {
        return type == CellType::Floor;
    }

private:
    CellType type;
};