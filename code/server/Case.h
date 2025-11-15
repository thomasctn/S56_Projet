#pragma once

enum class CaseType {
    Floor,
    Wall,
    Hut
};

struct Case {
    CaseType type;

    Case(CaseType t = CaseType::Floor) : type(t) {}
};
