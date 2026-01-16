#include "BotManager.h"
#include "BotController.h"
#include "Board.h"
#include "Player.h"
#include "InputQueue.h"

BotManager::BotManager(Game& g, InputQueue& q) : game(g), inputQueue(q) {}

// --- Graphe global ---
void BotManager::generateGraph(const Board& board) {
    int width = board.getWidth();
    int height = board.getHeight();

    graph.resize(height, std::vector<Node>(width));

    // Créer tous les noeuds
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            graph[y][x] = {x, y};
        }
    }

    // Connecter les voisins
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            auto& node = graph[y][x];

            CellType type = board.getCase(x, y).getType();
            if (type != CellType::Floor && type != CellType::Hut)
                continue;

            const std::vector<std::pair<int,int>> offsets = {{0,-1},{0,1},{-1,0},{1,0}};
            for (auto [dx, dy] : offsets) {
                int nx = x + dx;
                int ny = y + dy;
                if (!board.isInside(nx, ny)) continue;
                CellType nt = board.getCase(nx, ny).getType();
                if (nt != CellType::Floor && nt != CellType::Hut) continue;

                node.neighbors.push_back(&graph[ny][nx]);
            }
        }
    }
}

Node* BotManager::getNode(int x, int y) {
    if (y < 0 || y >= (int)graph.size()) return nullptr;
    if (x < 0 || x >= (int)graph[y].size()) return nullptr;
    return &graph[y][x];
}

// --- Gestion bots ---
void BotManager::registerBot(uint32_t id, std::unique_ptr<BotController> bot) {
    bots[id] = std::move(bot);
}

void BotManager::unregisterBot(uint32_t id) {
    bots.erase(id);
}

void BotManager::update() {
    for (auto& [botId, bot] : bots) {
        Player& p = game.getPlayerInfo(botId);

        if (!bot) continue;

        auto dirOpt = bot->update(game, *this); // on passe BotManager pour accès graphe et trace
        if (!dirOpt) continue;

        PlayerInput input;
        input.playerId = botId;
        input.dir = *dirOpt;

        inputQueue.push(input); 
    }
}
