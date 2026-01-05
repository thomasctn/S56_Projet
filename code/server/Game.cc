#include "Game.h"
#include <stdexcept>

Game::Game(int width, int height)
: plateau(width, height) {}

bool Game::canMove(uint32_t playerId, float newX, float newY) const {
    const auto& plateauRef = getPlateau();

    int gridX = static_cast<int>(newX) / 50;
    int gridY = static_cast<int>(newY) / 50;

    if (!plateauRef.isInside(gridX, gridY)) return false;
    if (!plateauRef.getCase(gridX, gridY).isWalkable()) return false;

    for (const auto& p : players) {
        if (p.id != playerId) {
            int px = static_cast<int>(p.x) / 50;
            int py = static_cast<int>(p.y) / 50;
            if (px == gridX && py == gridY) return false;
        }
    }

    return true;
}

void Game::requestMove(uint32_t playerId, Direction dir)
{
    constexpr float step = 50.0f;

    for (auto& p : players) {
        if (p.id == playerId) {
            float newX = p.x;
            float newY = p.y;

            switch (dir) {
            case Direction::Up:    newY -= step; break;
            case Direction::Down:  newY += step; break;
            case Direction::Left:  newX -= step; break;
            case Direction::Right: newX += step; break;
            }

            if (canMove(playerId, newX, newY)) {
                p.x = newX;
                p.y = newY;
            }
            return;
        }
    }
}

Player& Game::getPlayerInfo(uint32_t playerId) {
    for (auto& p : players) {
        if (p.id == playerId) return p;
    }
    throw std::runtime_error("Player not found");
}

void Game::addPlayer(uint32_t id, float x, float y) {
    players.emplace_back(id); // construit Player(id) directement dans le vecteur
    Player& p = players.back();
    p.x = x;
    p.y = y;
}

