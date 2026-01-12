#pragma once
#include "../common/Types.h"
#include "../common/Protocol.h"

class Case {
public:
    Case() : type(CellType::Floor) {}
    Case(CellType t) : type(t) {}

    CellType getType() const { return type; }

    bool isWalkable() const {
        return ((type == CellType::Floor) || (type == CellType::Hut)); // on peut marcher seulement sur le sol et la cabane
    }

    CaseCommon toCommonData() const {
        CaseCommon cc(type);
        return cc;
    }

private:
    CellType type;
};
