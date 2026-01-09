#include "BotController.h"
#include "Game.h"
#include "Player.h"

#include <random>
#include <vector>
#include <optional>
#include <iostream> // pour debug/log

static constexpr int VISION_RANGE = 4;
static constexpr float CHASE_DURATION = 5.0f;
static constexpr float MOVE_INTERVAL = 0.5f;
constexpr size_t CASE_SIZE = 50;

Direction opposite(Direction dir) {
    switch (dir) {
        case Direction::Up: return Direction::Down;
        case Direction::Down: return Direction::Up;
        case Direction::Left: return Direction::Right;
        case Direction::Right: return Direction::Left;
    }
    return dir; // fallback
}

std::optional<Direction> BotController::update(Game& game) {
    const auto& players = game.getPlayers();
    auto it = players.find(playerId);
    if (it == players.end()) {
        gf::Log::info("[BotController %u] Joueur introuvable\n", playerId);
        return std::nullopt;
    }

    auto& me = *(it->second);
    float time = static_cast<float>(game.getElapsedSeconds());

    // cooldown déplacement
    if (time - lastMoveTime < MOVE_INTERVAL) {
        gf::Log::info("[BotController %u] Attente cooldown %.2f\n", playerId, MOVE_INTERVAL - (time - lastMoveTime));
        return std::nullopt;
    }

    Board& board = game.getBoard();

    // Coordonnées grille
    int cx = static_cast<int>(me.x / CASE_SIZE);
    int cy = static_cast<int>(me.y / CASE_SIZE);

    // 1) Continuer direction actuelle si possible
    if (currentDir.has_value()) {
        int nx = cx, ny = cy;
        switch (*currentDir) {
            case Direction::Up:    ny -= 1; break;
            case Direction::Down:  ny += 1; break;
            case Direction::Left:  nx -= 1; break;
            case Direction::Right: nx += 1; break;
        }

        if (board.isInside(nx, ny)) {
            auto cellType = board.getCase(nx, ny).getType();
            if (cellType == CellType::Floor || cellType == CellType::Hut) {
                lastMoveTime = time;
                gf::Log::info("[BotController %u] Continuer direction actuelle %d vers (%d,%d)\n",
                              playerId, static_cast<int>(*currentDir), nx, ny);
                return currentDir;
            }
        }
        gf::Log::info("[BotController %u] Direction actuelle bloquée, réinitialisation\n", playerId);
        currentDir.reset();
    }

    // 2) Détection Pac-Man
    auto [pacX, pacY] = getPacManPosition(game);
    float dx = std::abs(me.x - pacX);
    float dy = std::abs(me.y - pacY);
    if ((dx + dy) <= VISION_RANGE && isVisible(me.x, me.y, pacX, pacY)) {
        memory.pacmanVisible = true;
        memory.lastSeenTime = time;
        memory.lastPacX = pacX;
        memory.lastPacY = pacY;
        memory.visitCount.clear();
        gf::Log::info("[BotController %u] Pac-Man détecté à (%.1f,%.1f)\n", playerId, pacX, pacY);
    }

    // 3) Mode chasse
    if (memory.pacmanVisible && time - memory.lastSeenTime <= CHASE_DURATION) {
        Direction dir = getDirectionTowards(me.x, me.y, memory.lastPacX, memory.lastPacY);

        int nx = cx, ny = cy;
        switch (dir) {
            case Direction::Up:    ny -= 1; break;
            case Direction::Down:  ny += 1; break;
            case Direction::Left:  nx -= 1; break;
            case Direction::Right: nx += 1; break;
        }

        if (board.isInside(nx, ny)) {
            auto cellType = board.getCase(nx, ny).getType();
            if (cellType == CellType::Floor || cellType == CellType::Hut) {
                currentDir = dir;
                lastMoveTime = time;
                gf::Log::info("[BotController %u] Chasse Pac-Man, direction %d vers (%d,%d)\n",
                              playerId, static_cast<int>(dir), nx, ny);
                return dir;
            }
        }
        gf::Log::info("[BotController %u] Chasse bloquée par mur\n", playerId);
    } else {
        memory.pacmanVisible = false;
    }

    // 4) Exploration
    memory.visitCount[{cx, cy}]++;
    gf::Log::info("[BotController %u] Exploration, position (%d,%d), visites %d\n",
                  playerId, cx, cy, memory.visitCount[{cx, cy}]);

    struct Candidate { Direction dir; int visits; int nx; int ny; };
    std::vector<Candidate> candidates;

    for (auto dir : {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
        // anti-retour immédiat
        if (currentDir.has_value() && dir == opposite(*currentDir))
            continue;

        int nx = cx, ny = cy;
        switch (dir) {
            case Direction::Up:    ny -= 1; break;
            case Direction::Down:  ny += 1; break;
            case Direction::Left:  nx -= 1; break;
            case Direction::Right: nx += 1; break;
        }

        if (!board.isInside(nx, ny)) continue;
        auto cellType = board.getCase(nx, ny).getType();
        if (cellType != CellType::Floor && cellType != CellType::Hut) continue;

        int visits = memory.visitCount[{nx, ny}];
        candidates.push_back({dir, visits, nx, ny});
        gf::Log::info("[BotController %u] Candidate direction %d vers (%d,%d), visites %d\n",
                      playerId, static_cast<int>(dir), nx, ny, visits);
    }

    if (candidates.empty()) {
        gf::Log::info("[BotController %u] Aucune direction possible\n", playerId);
        return std::nullopt;
    }

    // Choix de la meilleure direction (min visites) avec random si égalité
    int minVisits = candidates.front().visits;
    std::vector<Candidate> bests;
    for (auto& c : candidates) {
        if (c.visits < minVisits) {
            minVisits = c.visits;
            bests.clear();
            bests.push_back(c);
        } else if (c.visits == minVisits) {
            bests.push_back(c);
        }
    }

    // random parmi les meilleurs
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, bests.size() - 1);
    Candidate chosen = bests[dist(gen)];

    currentDir = chosen.dir;
    lastMoveTime = time;
    gf::Log::info("[BotController %u] Choisi direction %d vers (%d,%d) avec visites %d\n",
                  playerId, static_cast<int>(chosen.dir), chosen.nx, chosen.ny, chosen.visits);

    return chosen.dir;
}








std::pair<float,float> BotController::getPacManPosition(const Game& game) {
    for (auto& [id, p] : game.getPlayers()) {
        if (p->role == PlayerRole::PacMan)
            return {p->x, p->y};
    }
    return {0.f, 0.f};
}


bool BotController::isVisible(float fromX, float fromY, float toX, float toY) {
    return (fromX == toX || fromY == toY);
}

Direction BotController::getDirectionTowards(
    float fromX, float fromY,
    float toX, float toY
) {
    float dx = toX - fromX;
    float dy = toY - fromY;

    if (std::abs(dx) > std::abs(dy)) {
        return (dx > 0) ? Direction::Right : Direction::Left;
    } else {
        return (dy > 0) ? Direction::Down : Direction::Up;
    }
}

bool BotController::canGoTo(
    const Game& game,
    const Player& me,
    Direction dir
) {
    int tx = static_cast<int>(me.x);
    int ty = static_cast<int>(me.y);

    switch (dir) {
        case Direction::Up:    ty -= 1; break;
        case Direction::Down:  ty += 1; break;
        case Direction::Left:  tx -= 1; break;
        case Direction::Right: tx += 1; break;
    }

    const Board& board = game.getBoard();

    if (!board.isInside(tx, ty))
        return false;

    if (!board.isWalkable(tx, ty))
        return false;

    return game.canMove(me.id, tx, ty);
}
