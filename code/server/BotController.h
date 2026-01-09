#pragma once
#include "Controller.h"
#include "../common/Types.h"
#include "Board.h"

class BotController : public Controller {
public:
    BotController(uint32_t id) : playerId(id) {}

    std::optional<Direction> update(Game& game) override;

private:
    uint32_t playerId;
    Direction lastDir = Direction::Up;
    std::pair<float,float> getPacManPosition(const Game& game);
    bool isVisible(float fromX, float fromY, float toX, float toY);
    Direction getDirectionTowards(float fromX, float fromY, float toX, float toY);
    bool canGoTo(const Game& game,const Player& me,Direction dir);

std::optional<Direction> currentDir;
float lastMoveTime = 0.f;
static constexpr float MOVE_INTERVAL = 0.5f;


};
