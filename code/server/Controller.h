#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <unordered_map>
#include "Game.h"
#include "Constants.h"

struct Node {
    int x, y;
    std::vector<Node*> neighbors;
};

class Controller {
public:
    uint32_t playerId;
    Direction dir;

    virtual ~Controller() = default;
    virtual std::optional<Direction> update(Game& game) = 0;

    // --- Graphe global ---
    std::vector<std::vector<Node>> graph;

    void generateGraph(const Board& board);
    Node* getNode(int x, int y);
};
