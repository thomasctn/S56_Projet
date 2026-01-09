#include "BotController.h"
#include "Game.h"
#include "Player.h"

#include <random>
#include <vector>
#include <optional>
#include <iostream> // pour debug/log


std::optional<Direction> BotController::update(Game& game) {
    const auto& players = game.getPlayers();
    auto it = players.find(playerId);
    if (it == players.end()) {
        return std::nullopt; // joueur non trouvé
    }

    auto& me = *(it->second);
    auto [pacX, pacY] = getPacManPosition(game);

    // Si Pac-Man visible dans le voisinage
    if (isVisible(me.x, me.y, pacX, pacY)) {
        return getDirectionTowards(me.x, me.y, pacX, pacY);
    }

    // Sinon, choisir une direction aléatoire accessible
    std::vector<Direction> options;
    for (auto dir : {Direction::Up, Direction::Down, Direction::Left, Direction::Right}) {
        float targetX = me.x;
        float targetY = me.y;

        switch (dir) {
            case Direction::Up:    targetY -= 1.0f; break;
            case Direction::Down:  targetY += 1.0f; break;
            case Direction::Left:  targetX -= 1.0f; break;
            case Direction::Right: targetX += 1.0f; break;
        }

        if (game.canMove(me.id, targetX, targetY)) {
            options.push_back(dir);
        }
    }



    if (options.empty())
        return std::nullopt; // aucun mouvement possible

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, options.size() - 1);
    return options[dis(gen)];
}


std::pair<float,float> BotController::getPacManPosition(const Game& game) {
    const auto& players = game.getPlayers();
    for (auto& [id, p] : players) {
        if (p->role == PlayerRole::PacMan)
            return {p->x, p->y};
    }
    return {0.0f, 0.0f};
}


bool BotController::isVisible(float fromX, float fromY, float toX, float toY) {
    return (fromX == toX || fromY == toY);
}

Direction BotController::getDirectionTowards(float fromX, float fromY, float toX, float toY) {
    if (fromX < toX) return Direction::Right;
    if (fromX > toX) return Direction::Left;
    if (fromY < toY) return Direction::Down;
    return Direction::Up;
}

