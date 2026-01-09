#pragma once

#include <unordered_set>
#include <cstdint>

class Game;
class InputQueue;

class BotManager {
public:
    BotManager(Game& game, InputQueue& queue);

    void update();

    void registerBot(uint32_t id) {
        botIds.insert(id);
    }

private:
    Game& game;
    InputQueue& inputQueue;
    std::unordered_set<uint32_t> botIds;
};
