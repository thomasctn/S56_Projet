#pragma once
#include <optional>
#include <utility>
#include <vector>
#include <unordered_map>
#include "Game.h"
#include "BotManager.h"

class BotController {
public:
    explicit BotController(uint32_t id) : playerId(id) {}

    // Recevoir BotManager pour accéder au graphe et à la trace globale
    std::optional<Direction> update(Game& game, BotManager& manager);

private:
    uint32_t playerId;

    struct Memory {
        bool pacmanVisible = false;
        float lastPacX = 0.f;
        float lastPacY = 0.f;
        float lastSeenTime = 0.f;

        std::unordered_map<std::pair<int,int>, int, pair_hash> visitCount;
    } memory;

    std::optional<Direction> currentDir;
    float lastMoveTime = 0.f;

    std::pair<float,float> getPacManPosition(const Game& game) const;
    bool isVisible(float fromX, float fromY, float toX, float toY) const;
    Direction getDirectionTowards(float fromX, float fromY, float toX, float toY) const;
    std::optional<std::pair<int,int>> lastSeenPacMan;
    bool hasLineOfSight(const Board& board, int x0, int y0, int x1, int y1) const;
};
