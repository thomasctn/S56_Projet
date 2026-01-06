#pragma once
#include "../common/Types.h"
#include "../common/Protocol.h"

class Case {
public:
    Case() : type(CellType::Floor), pacGomme(false) {}
    Case(CellType t) : type(t), pacGomme(false) {}

    CellType getType() const { return type; }

    bool isWalkable() const {
        return type == CellType::Floor; // on peut marcher seulement sur le sol
    }

    // --- Pac-gomme ---
    bool hasPacGomme() const { return pacGomme; }
    void placePacGomme() { if(type == CellType::Floor) pacGomme = true; }
    void removePacGomme() { pacGomme = false; }

    CaseCommon toCommonData() const {
        CaseCommon cc(type);
        return cc;
    }

private:
    CellType type;
    bool pacGomme;
};
