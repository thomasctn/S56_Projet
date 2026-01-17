#include "BotController.h"
#include "BotManager.h"
#include "Game.h"
#include "Player.h"
#include "../common/Constants.h"

#include <optional>
#include <limits>
#include <cstdlib>

BotController::BotController(uint32_t id)
: playerId(id) {}

// --------------------------------------------------
// Scoring d’un noeud du graphe
// --------------------------------------------------
static float scoreNode(
    const std::vector<Trace>& traces,
    uint32_t selfId
) {
    float score = 0.f;

    for (const Trace& t : traces) {
        if (t.type == TraceType::PacMan) {
            score += PACMAN_ATTRACTION * t.intensity;
        }
        else { // Ghost
            if (t.ownerId == selfId) {
                score -= SELF_GHOST_REPULSION * t.intensity;
            } else {
                score -= OTHER_GHOST_REPULSION * t.intensity;
            }
        }
    }

    return score;
}

// --------------------------------------------------
// BotController::update
// --------------------------------------------------
std::optional<Direction>
BotController::update(Game& game, BotManager& manager) {

    if (!game.isGameStarted())
        return std::nullopt;

    // --- Récupération du joueur ---
    auto& players = game.getPlayers();
    auto it = players.find(playerId);
    if (it == players.end())
        return std::nullopt;

    Player& me = *(it->second);

    // --- Position courante sur le graphe ---
    int gx = static_cast<int>(me.x / CASE_SIZE);
    int gy = static_cast<int>(me.y / CASE_SIZE);

    Node* current = manager.getNode(gx, gy);
    if (!current || current->neighbors.empty())
        return std::nullopt;

    // --- Choix du meilleur voisin ---
    Node* bestNode = nullptr;
    float bestScore = -std::numeric_limits<float>::infinity();

    for (Node* neighbor : current->neighbors) {

        const auto& traces =
            manager.getTraces().get(neighbor);

        float score = scoreNode(traces, playerId);

        // bruit léger pour casser les cycles
        score += ((std::rand() % 100) / 100.f - 0.5f) * RANDOM_NOISE;

        if (score > bestScore) {
            bestScore = score;
            bestNode = neighbor;
        }
    }

    if (!bestNode)
        return std::nullopt;

    // --- Conversion noeud -> direction ---
    return getDirectionTowards(
        me.x, me.y,
        bestNode->x * CASE_SIZE,
        bestNode->y * CASE_SIZE
    );
}

// --------------------------------------------------
// Helpers
// --------------------------------------------------
Direction
BotController::getDirectionTowards(
    float fromX, float fromY,
    float toX,   float toY
) const {
    if (fromX < toX) return Direction::Right;
    if (fromX > toX) return Direction::Left;
    if (fromY < toY) return Direction::Down;
    return Direction::Up;
}
