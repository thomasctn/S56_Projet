#include "Game.h"
#include <stdexcept> // pour std::runtime_error

Game::Game(int width, int height)
: plateau(width, height) {}

bool Game::canMove(uint32_t playerId, float newX, float newY) const {
    const auto& plateauRef = getPlateau();
    const auto& playersRef = players;

    int gridX = static_cast<int>(newX) / 50;
    int gridY = static_cast<int>(newY) / 50;

    // vérifier mur / hut
    if (!plateauRef.getCase(gridX, gridY).isWalkable()) return false;

    // vérifier collision avec autre joueur
    for (auto& p : playersRef) {
        if (p.id != playerId) {
            int px = static_cast<int>(p.x) / 50;
            int py = static_cast<int>(p.y) / 50;
            if (px == gridX && py == gridY) return false;
        }
    }

    return true;
}

void Game::movePlayer(uint32_t playerId, float newX, float newY) {
    if (!canMove(playerId, newX, newY)) return;

    for (auto& p : players) {
        if (p.id == playerId) {
            p.x = newX;
            p.y = newY;
            break;
        }
    }
}

Player& Game::getPlayerInfo(uint32_t playerId) {
    for (auto& p : players) {
        if (p.id == playerId)
            return p;
    }
    throw std::runtime_error("Player not found");
}

void Game::addPlayer(uint32_t id, float x, float y) {
    players.push_back({id, x, y});
}