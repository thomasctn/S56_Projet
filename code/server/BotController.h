#pragma once
#include "Controller.h"

class BotController : public Controller {
public:
    BotController(uint32_t id) : playerId(id) {}

    std::optional<Direction> update(Game& game) override;

private:
    uint32_t playerId;

    std::pair<float,float> getPacManPosition(const Game& game);
    bool isVisible(float fromX, float fromY, float toX, float toY);
    Direction getDirectionTowards(float fromX, float fromY, float toX, float toY);
};
