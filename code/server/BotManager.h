#pragma once

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <optional>
#include <memory>
#include "Game.h"
#include "Constants.h"

struct Node {
    int x, y;
    std::vector<Node*> neighbors;
};

struct pair_hash {
    inline std::size_t operator()(const std::pair<int,int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

class BotController; // forward declaration
class InputQueue;

class BotManager {
public:
    BotManager(Game& game, InputQueue& queue);

    void update(); // appelle update() de chaque bot et injecte dans InputQueue

    void registerBot(uint32_t id, std::unique_ptr<BotController> bot);
    void unregisterBot(uint32_t id);

    // --- Graphe global (fusion Controller) ---
    std::vector<std::vector<Node>> graph;
    void generateGraph(const Board& board);
    Node* getNode(int x, int y);

    // Trace globale partagée par tous les bots
    std::unordered_map<std::pair<int,int>, int, pair_hash> globalVisitCount;

private:
    Game& game;
    InputQueue& inputQueue;
    std::unordered_map<uint32_t, std::unique_ptr<BotController>> bots;
};
