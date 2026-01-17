#pragma once

#include <optional>
#include <vector>
#include "../common/Types.h"


// forward declarations (important)
class Game;
class BotManager;
struct Node;
struct Trace;

// --------------------------------------------------
// BotController
// --------------------------------------------------
class BotController {
public:
    explicit BotController(uint32_t playerId);

    // appelée par BotManager
    std::optional<Direction> update(Game& game, BotManager& manager);

private:
    Direction getDirectionTowards(
        float fromX, float fromY,
        float toX,   float toY
    ) const;

private:
    uint32_t playerId;
};
