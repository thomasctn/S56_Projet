#pragma once

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>

#include "Game.h"
#include "Constants.h"


struct PairHash {
    std::size_t operator()(const std::pair<int,int>& p) const noexcept {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

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


struct BotMemory {
    std::unordered_map<std::pair<int,int>, int, PairHash> visitCount;
    bool pacmanVisible = false;
    float lastSeenTime = 0.f;
    float lastPacX = 0.f;
    float lastPacY = 0.f;
};


extern BotMemory memory;
