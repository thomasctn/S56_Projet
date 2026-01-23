#include "BotController.h"
#include "BotManager.h"
#include "Game.h"
#include "Player.h"
#include "../common/Constants.h"

#include <optional>
#include <limits>
#include <cstdlib>
#include <queue>
#include <unordered_set>
#include <utility>


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
std::optional<Direction> BotController::update(Game& game, BotManager& manager) {

    if (!game.isGameStarted())
        return std::nullopt;

    // --- Récupération du joueur ---
    auto& players = game.getPlayers();
    auto it = players.find(playerId);
    if (it == players.end())
        return std::nullopt;

    Player& me = *(it->second);

    int gx = static_cast<int>(me.x / CASE_SIZE);
    int gy = static_cast<int>(me.y / CASE_SIZE);

    Node* current = manager.getNode(gx, gy);
    if (!current || current->neighbors.empty())
        return std::nullopt;

    // --- Récupération de Pac-Man ---
    Player* pacMan = nullptr;
    for (auto& [id, p] : players) {
        if (p->getRole() == PlayerRole::PacMan) {
            pacMan = p.get();
            break;
        }
    }

    // --- PRIORITÉ 1 : Fuite vers la cabane si Pac-Man chasseur ---
    if (pacMan && !pacMan->isVunerable && me.getRole() == PlayerRole::Ghost) {
        // Calcul du centre de la map (cabane 3x3)
        int centerX = game.getBoard().getWidth() / 2;
        int centerY = game.getBoard().getHeight() / 2;

        Node* centerNode = manager.getNode(centerX, centerY);
        if (centerNode) {
            // Utilisation de BFS pour trouver le prochain noeud vers la cabane
            Node* nextStep = getNextNodeTowards(current, centerNode);
            if (nextStep) {
                return getDirectionTowards(me.x, me.y,
                                        nextStep->x * CASE_SIZE,
                                        nextStep->y * CASE_SIZE);
            }
        }
    }



    // --- PRIORITÉ 2 : Pac-Man visible dans le rayon ---
    if (pacMan) {
        Node* pacmanNode = getPacmanNode(game, manager);
        if (pacmanNode) {
            Node* seen = findPacmanInSight(current, pacmanNode, VISION_RANGE_BOTS);
            if (seen) {
                // Va vers le premier voisin sur le chemin
                Node* nextStep = nullptr;
                for (Node* n : current->neighbors) {
                    if (n->x == pacmanNode->x && n->y == pacmanNode->y) {
                        nextStep = n;
                        break;
                    }
                }
                if (!nextStep) {
                    // Si Pac-Man pas exactement voisin, prend le noeud le plus proche
                    float minDist = std::numeric_limits<float>::infinity();
                    for (Node* n : current->neighbors) {
                        float dx = static_cast<float>(n->x - pacmanNode->x);
                        float dy = static_cast<float>(n->y - pacmanNode->y);
                        float d = dx * dx + dy * dy;
                        if (d < minDist) {
                            minDist = d;
                            nextStep = n;
                        }
                    }
                }

                if (nextStep)
                    return getDirectionTowards(me.x, me.y,
                                               nextStep->x * CASE_SIZE,
                                               nextStep->y * CASE_SIZE);
            }
        }
    }

    // --- PRIORITÉ 3 : choisir le meilleur voisin selon traces ---
    Node* bestNode = nullptr;
    float bestScore = -std::numeric_limits<float>::infinity();

    for (Node* neighbor : current->neighbors) {
        const auto& traces = manager.getTraces().get(neighbor);
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

    return getDirectionTowards(me.x, me.y,
                               bestNode->x * CASE_SIZE,
                               bestNode->y * CASE_SIZE);
}



// --------------------------------------------------
// Helpers
// --------------------------------------------------
Direction
BotController::getDirectionTowards(
    float fromX, float fromY,
    float toX,   float toY
) const {
    float dx = toX - fromX;
    float dy = toY - fromY;

    // Priorité sur l'axe le plus éloigné
    if (std::abs(dx) > std::abs(dy)) {
        return dx > 0 ? Direction::Right : Direction::Left;
    } else if (std::abs(dy) > 0) {
        return dy > 0 ? Direction::Down : Direction::Up;
    }

    return Direction::None; // on est déjà dessus
}


Node* BotController::getPacmanNode(Game& game, BotManager& manager) {
    for (auto& [id, p] : game.getPlayers()) {
    if (p->getRole() == PlayerRole::PacMan) {
            int px = static_cast<int>(p->x / CASE_SIZE);
            int py = static_cast<int>(p->y / CASE_SIZE);
            return manager.getNode(px, py);
        }
    }
    return nullptr;
}

Node* BotController::findPacmanInSight(
    Node* start,
    Node* pacmanNode,
    int maxDepth
) {
    if (!start || !pacmanNode) return nullptr;

    std::queue<std::pair<Node*, int>> q;
    std::unordered_set<Node*> visited;

    q.push({ start, 0 });
    visited.insert(start);

    while (!q.empty()) {
        auto [node, depth] = q.front();
        q.pop();

        if (node == pacmanNode)
            return node;

        if (depth >= maxDepth)
            continue;

        for (Node* n : node->neighbors) {
            if (!visited.count(n)) {
                visited.insert(n);
                q.push({ n, depth + 1 });
            }
        }
    }

    return nullptr;
}

Node* BotController::getNextNodeTowards(Node* start, Node* target) {
    if (!start || !target) return nullptr;

    std::queue<Node*> q;
    std::unordered_map<Node*, Node*> parent;
    std::unordered_set<Node*> visited;

    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        Node* current = q.front(); q.pop();
        if (current == target) break;

        for (Node* n : current->neighbors) {
            if (!visited.count(n)) {
                visited.insert(n);
                parent[n] = current;
                q.push(n);
            }
        }
    }

    // Si on n’a pas trouvé la cible
    if (!parent.count(target)) return nullptr;

    // remonte jusqu’au voisin direct de start
    Node* step = target;
    while (parent[step] != start) {
        step = parent[step];
    }
    return step;
}
