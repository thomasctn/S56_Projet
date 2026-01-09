#pragma once
#include "Controller.h"
#include "Game.h"
#include <optional>

class BotController : public Controller {
public:
    std::optional<Direction> update(Game& game) override {
        return Direction::Up; // exemple simple
    }
};
