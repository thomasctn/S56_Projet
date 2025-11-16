#pragma once

enum class CellType {
    Floor,
    Wall,
    Hut
};

class Case {
public:
    Case(CellType type = CellType::Floor) : type(type) {}

    CellType getType() const { return type; }
    void setType(CellType t) { type = t; }

private:
    CellType type;
};
