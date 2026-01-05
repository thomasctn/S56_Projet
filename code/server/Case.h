#pragma once
#include "../common/Types.h"
#include "../common/Protocol.h"

class Case {
public:
    Case() : type(CellType::Floor) {}
    Case(CellType t) : type(t) {}

    CellType getType() const { return type; }

    bool isWalkable() const {
        return type == CellType::Floor;
    }
    CaseCommon toCommonData() const {return CaseCommon(type);}

private:
    CellType type;
};