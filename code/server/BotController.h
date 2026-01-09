#pragma once
#include "Controller.h"
#include "Game.h"
#include <optional>
#include <utility>
#include <vector>
#include <unordered_map>

struct pair_hash {
    inline std::size_t operator()(const std::pair<int,int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

class BotController : public Controller {
public:
    explicit BotController(uint32_t id) : playerId(id) {}

    std::optional<Direction> update(Game& game) override;

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
