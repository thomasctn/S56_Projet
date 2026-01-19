#include "BotManager.h"
#include "BotController.h"
#include "Board.h"
#include "Game.h"
#include "Player.h"
#include "InputQueue.h"
#include "../common/Constants.h"

// ---------------- Constructor ----------------
BotManager::BotManager(Game& g, InputQueue& q)
: game(g), inputQueue(q) {}

// ---------------- Graph ----------------
void BotManager::generateGraph(const Board& board) {
    int width = board.getWidth();
    int height = board.getHeight();

    graph.resize(height, std::vector<Node>(width));

    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            graph[y][x] = {x, y};

    const std::vector<std::pair<int,int>> offsets = {
        {0,-1},{0,1},{-1,0},{1,0}
    };

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {

            CellType type = board.getCase(x, y).getType();
            if (type != CellType::Floor && type != CellType::Hut)
                continue;

            for (auto [dx, dy] : offsets) {
                int nx = x + dx;
                int ny = y + dy;
                if (!board.isInside(nx, ny)) continue;

                CellType nt = board.getCase(nx, ny).getType();
                if (nt != CellType::Floor && nt != CellType::Hut)
                    continue;

                graph[y][x].neighbors.push_back(&graph[ny][nx]);
            }
        }
    }
}

Node* BotManager::getNode(int x, int y) {
    if (y < 0 || y >= (int)graph.size()) return nullptr;
    if (x < 0 || x >= (int)graph[y].size()) return nullptr;
    return &graph[y][x];
}

// ---------------- Bots ----------------
void BotManager::registerBot(uint32_t id, std::unique_ptr<BotController> bot) {
    bots[id] = std::move(bot);
}

void BotManager::unregisterBot(uint32_t id) {
    bots.erase(id);
}

// ---------------- Traces ----------------
void BotManager::updateTraces() {
    for (auto& [id, p] : game.getPlayers()) {
        int gx = (int)(p->x / CASE_SIZE);
        int gy = (int)(p->y / CASE_SIZE);

        Node* node = getNode(gx, gy);
        if (!node) continue;

        Trace t;
        t.ownerId = id;
        t.intensity = 1.0f;
        t.type = (p->role == PlayerRole::PacMan)
                   ? TraceType::PacMan
                   : TraceType::Ghost;

        traceMap.add(node, t);
    }

    traceMap.decay();
}

const GraphTraceMap& BotManager::getTraces() const {
    return traceMap;
}

// ---------------- Update ----------------
void BotManager::update() {
    for (auto& [botId, bot] : bots) {
        Player& p = game.getPlayerInfo(botId);
        (void)p;

        auto dirOpt = bot->update(game, *this);
        if (!dirOpt) continue;

        PlayerInput input;
        input.playerId = botId;
        input.dir = *dirOpt;
        inputQueue.push(input);
    }
}
