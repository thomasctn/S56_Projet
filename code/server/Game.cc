#include "Game.h"

Game::Game(int width, int height) : plateau(width, height) {}


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
    players.emplace_back(id);
    Player& p = players.back();
    p.x = x;
    p.y = y;
}

void Game::startChrono() {
    chronoStart = std::chrono::steady_clock::now();
}

double Game::getElapsedSeconds() const {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = now - chronoStart;
    return elapsed.count();
}

void Game::resetChrono() {
    chronoStart = std::chrono::steady_clock::now();
}

void Game::startGameLoop(int tickMs_) {
    tickMs = tickMs_;
    running = true;
    startChrono();

    gameThread = std::thread([this]() {
        int lastLoggedSecond = -1; // aucune seconde affichée au début

        while (running) {
            auto loopStart = std::chrono::steady_clock::now();

            // --- Ici la logique de mise à jour ---
            double elapsed = getElapsedSeconds();
            int currentSecond = static_cast<int>(elapsed);

            // Log uniquement si on change de seconde
            if (currentSecond != lastLoggedSecond) {
                lastLoggedSecond = currentSecond;
                gf::Log::info("Temps restant: %d secondes\n", 300 - currentSecond);
            }

            // Pause pour le prochain tick
            auto loopEnd = std::chrono::steady_clock::now();
            std::chrono::duration<double, std::milli> loopDuration = loopEnd - loopStart;
            int sleepMs = tickMs - static_cast<int>(loopDuration.count());
            if (sleepMs > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
        }
    });
}


void Game::stopGameLoop() {
    running = false;
    if (gameThread.joinable())
        gameThread.join();
}

