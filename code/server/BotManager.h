#pragma once

#include <unordered_map>
#include <memory>
#include <vector>
#include "../common/Constants.h"

#include "GraphTraceMap.h"

class Game;
class InputQueue;
class Board;
class BotController;

struct Node {
    int x, y;
    std::vector<Node*> neighbors;
};

class BotManager {
public:
    BotManager(Game& game, InputQueue& queue);

    void generateGraph(const Board& board);
    Node* getNode(int x, int y);

    void registerBot(uint32_t id, std::unique_ptr<BotController> bot);
    void unregisterBot(uint32_t id);

    void update();
    void updateTraces(float dt);

    GraphTraceMap& getTraces() { return traceMap; }
    const GraphTraceMap& getTraces() const;

private:
    Game& game;
    InputQueue& inputQueue;

    std::unordered_map<uint32_t, std::unique_ptr<BotController>> bots;
    std::vector<std::vector<Node>> graph;

    GraphTraceMap traceMap;
};
