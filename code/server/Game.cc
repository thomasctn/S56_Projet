#include "Game.h"

#define T_GAME 10

Game::Game(int width, int height) : plateau(width, height) {}


bool Game::canMove(uint32_t playerId, float newX, float newY) const {
    const auto& plateauRef = getPlateau();

    int gridX = static_cast<int>(newX) / 50;
    int gridY = static_cast<int>(newY) / 50;

    if (!plateauRef.isInside(gridX, gridY)) return false;
    if (!plateauRef.getCase(gridX, gridY).isWalkable()) return false;

    // Récupère le joueur courant
    auto itCurrent = players.find(playerId);
    if (itCurrent == players.end()) return false;
    const Player& currentPlayer = *itCurrent->second;

    for (const auto& [id, playerPtr] : players) {
        if (id == playerId) continue;

        const Player& other = *playerPtr;

        int px = static_cast<int>(other.x) / 50;
        int py = static_cast<int>(other.y) / 50;

        // Fantômes peuvent marcher sur PacMan
        if (px == gridX && py == gridY) {
            if (currentPlayer.getRole() == PlayerRole::Ghost &&
                other.getRole() == PlayerRole::PacMan) {
                continue; // autorisé
            }
            return false; // sinon bloqué
        }
    }

    return true;
}



void Game::requestMove(uint32_t playerId, Direction dir) {
    constexpr float step = 50.0f;

    auto it = players.find(playerId);
    if (it == players.end()) return;

    Player& p = *it->second;

    float newX = p.x;
    float newY = p.y;

    switch (dir) {
        case Direction::Up:    newY -= step; break;
        case Direction::Down:  newY += step; break;
        case Direction::Left:  newX -= step; break;
        case Direction::Right: newX += step; break;
    }

    if (!canMove(playerId, newX, newY)) return;

    // Déplacement effectif
    p.x = newX;
    p.y = newY;

    // --- Gestion collisions joueurs ---
    for (auto& [otherId, otherPtr] : players) {
        if (otherId == playerId) continue;

        Player& other = *otherPtr;

        int px = static_cast<int>(other.x) / 50;
        int py = static_cast<int>(other.y) / 50;
        int cx = static_cast<int>(p.x) / 50;
        int cy = static_cast<int>(p.y) / 50;

        if (px == cx && py == cy) {
            // Si c'est un fantôme qui marche sur PacMan
            p.eat(false, &other);
            other.eat(false, &p); // dans l'autre sens si PacMan marche sur fantôme mais pas possible normalement
        }
    }
}



Player& Game::getPlayerInfo(uint32_t playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Player not found");
    }
    return *it->second;
}



void Game::addPlayer(uint32_t id, float x, float y) {
    auto player = std::make_unique<Player>(id, PlayerRole::Spectator);
    player->x = x;
    player->y = y;

    players.emplace(id, std::move(player));
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
                if (T_GAME - currentSecond <=0){
                    gf::Log::info("Partie terminé !\n");
                    break;
                }
                lastLoggedSecond = currentSecond;
                gf::Log::info("Temps restant: %d secondes\n", T_GAME - currentSecond);
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

void Game::spawnPlayer(Player& p) {
    if (p.getRole() == PlayerRole::Ghost) {
        // coordonnées de la cabane fantôme
        p.x = (plateau.getWidth() / 2) * 50.0f;
        p.y = (plateau.getHeight() / 2) * 50.0f;
    } else if (p.getRole() == PlayerRole::PacMan) {
        // spawn PacMan dans une zone libre (exemple : coin en haut à gauche)
        p.x = 50.0f;
        p.y = 50.0f;
    } else {
        // Spectator spawn
        p.x = 30.0f;
        p.y = 30.0f;
    }
}
