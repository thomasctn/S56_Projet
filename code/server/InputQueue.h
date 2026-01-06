#pragma once
#include <queue>
#include <mutex>
#include <optional>
#include "Game.h"

struct PlayerInput {
    uint32_t playerId;
    Direction dir;
};

class InputQueue {
private:
    std::queue<PlayerInput> queue;
    std::mutex mtx;

public:
    void push(const PlayerInput& input) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(input);
    }

    std::optional<PlayerInput> pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) return std::nullopt;
        PlayerInput input = queue.front();
        queue.pop();
        return input;
    }
};
