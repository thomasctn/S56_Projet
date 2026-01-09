
#pragma once
#include <cstdint>
#include <optional>
#include "Game.h"
#include "Constants.h"

class Controller {
public:
    uint32_t playerId;
    Direction dir;

    virtual ~Controller() = default;
    virtual std::optional<Direction> update(Game& game) = 0;
};


struct Input {
    uint32_t playerId;
    Direction dir;
};

