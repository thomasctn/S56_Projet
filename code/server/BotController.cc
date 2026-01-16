#include "BotController.h"
#include "Game.h"
#include "Player.h"
#include "../common/Constants.h"
#include <queue>
#include <algorithm>

// -------------------- A* utilitaire --------------------
struct NodeDist {
    Node* node;
    float cost;
    float heuristic;
    Node* parent;
};

std::vector<Node*> reconstructPath(Node* goal, std::unordered_map<Node*, Node*>& cameFrom) {
    std::vector<Node*> path;
    Node* current = goal;
    while (current) {
        path.push_back(current);
        auto it = cameFrom.find(current);
        if (it == cameFrom.end()) break;
        current = it->second;
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<Node*> findPath(Node* start, Node* goal) {
    if (!start || !goal) return {};

    std::queue<Node*> frontier;
    frontier.push(start);

    std::unordered_map<Node*, Node*> cameFrom;
    std::unordered_map<Node*, int> visited;
    visited[start] = 1;

    while (!frontier.empty()) {
        Node* current = frontier.front();
        frontier.pop();

        if (current == goal) {
            return reconstructPath(goal, cameFrom);
        }

        for (Node* neighbor : current->neighbors) {
            if (visited[neighbor]) continue;
            frontier.push(neighbor);
            visited[neighbor] = 1;
            cameFrom[neighbor] = current;
        }
    }

    return {};
}

// -------------------- BotController --------------------
std::optional<Direction> BotController::update(Game& game, BotManager& manager) {
    auto& players = game.getPlayers();
    auto it = players.find(playerId);
    if (it == players.end()) return std::nullopt;

    auto& me = *(it->second);

    if (!game.isGameStarted()) return std::nullopt;

    int gx = static_cast<int>(me.x / CASE_SIZE);
    int gy = static_cast<int>(me.y / CASE_SIZE);

    auto [pacX, pacY] = getPacManPosition(game);
    int pgx = static_cast<int>(pacX / CASE_SIZE);
    int pgy = static_cast<int>(pacY / CASE_SIZE);

    // --- Chasse Pac-Man ---
    if (hasLineOfSight(game.getBoard(), gx, gy, pgx, pgy)) {
        lastSeenPacMan = {pgx, pgy};

        Node* start = manager.getNode(gx, gy);
        Node* goal  = manager.getNode(pgx, pgy);

        if (start && goal) {
            auto path = findPath(start, goal);
            if (path.size() > 1) {
                Node* next = path[1];
                return getDirectionTowards(me.x, me.y, next->x * CASE_SIZE, next->y * CASE_SIZE);
            }
        }
    }

    // --- Dernière position vue ---
    if (lastSeenPacMan) {
        int lx = lastSeenPacMan->first;
        int ly = lastSeenPacMan->second;

        Node* start = manager.getNode(gx, gy);
        Node* goal  = manager.getNode(lx, ly);
        if (start && goal) {
            auto path = findPath(start, goal);
            if (path.size() > 1) {
                Node* next = path[1];
                return getDirectionTowards(me.x, me.y, next->x * CASE_SIZE, next->y * CASE_SIZE);
            } else {
                lastSeenPacMan = std::nullopt;
            }
        }
    }

    // --- Exploration ---
    Node* currentNode = manager.getNode(gx, gy);
    if (!currentNode) return std::nullopt;

    Node* bestNeighbor = nullptr;
    int minVisits = 1e6;

    for (Node* neighbor : currentNode->neighbors) {
        int visits = manager.globalVisitCount[{neighbor->x, neighbor->y}];
        if (visits < minVisits) {
            minVisits = visits;
            bestNeighbor = neighbor;
        }
    }

    if (!bestNeighbor) return std::nullopt;

    manager.globalVisitCount[{bestNeighbor->x, bestNeighbor->y}]++;
    return getDirectionTowards(me.x, me.y, bestNeighbor->x * CASE_SIZE, bestNeighbor->y * CASE_SIZE);
}

// -------------------- Helpers --------------------
std::pair<float,float> BotController::getPacManPosition(const Game& game) const {
    for (auto& [id, p] : game.getPlayers()) {
        if (p->role == PlayerRole::PacMan)
            return {p->x, p->y};
    }
    return {0.f, 0.f};
}

bool BotController::isVisible(float fromX, float fromY, float toX, float toY) const {
    int fx = static_cast<int>(fromX / CASE_SIZE);
    int fy = static_cast<int>(fromY / CASE_SIZE);
    int tx = static_cast<int>(toX / CASE_SIZE);
    int ty = static_cast<int>(toY / CASE_SIZE);
    return (fx == tx || fy == ty);
}

Direction BotController::getDirectionTowards(float fromX, float fromY, float toX, float toY) const {
    if (fromX < toX) return Direction::Right;
    if (fromX > toX) return Direction::Left;
    if (fromY < toY) return Direction::Down;
    return Direction::Up;
}

bool BotController::hasLineOfSight(const Board& board, int x0, int y0, int x1, int y1) const {
    int dx = (x1 > x0) ? 1 : (x1 < x0) ? -1 : 0;
    int dy = (y1 > y0) ? 1 : (y1 < y0) ? -1 : 0;

    int cx = x0;
    int cy = y0;

    if (cx == x1 && cy == y1) return true;

    while (cx != x1 || cy != y1) {
        if (board.getCase(cx, cy).getType() == CellType::Wall)
            return false;

        if (cx != x1) cx += dx;
        if (cy != y1) cy += dy;
    }

    return true;
}
