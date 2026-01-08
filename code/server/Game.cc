#include "Game.h"
#include "ServerNetwork.h"
#include "Room.h"



Game::Game(int width, int height) : board(width, height) {
    board.placeRandomPacGommes(20);
}


bool Game::canMove(uint32_t playerId, float newX, float newY) const {
    const auto& boardRef = getBoard();

    int gridX = static_cast<int>(newX) / 50;
    int gridY = static_cast<int>(newY) / 50;

    if (!boardRef.isInside(gridX, gridY)) return false;
    if (!boardRef.getCase(gridX, gridY).isWalkable()) return false;

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



bool Game::requestMove(uint32_t playerId, Direction dir) {
    if (!gameStarted) return false;

    constexpr float step = 50.0f;

    auto it = players.find(playerId);
    if (it == players.end()) return false;

    Player& p = *it->second;

    float newX = p.x;
    float newY = p.y;

    switch (dir) {
        case Direction::Up:    newY -= step; break;
        case Direction::Down:  newY += step; break;
        case Direction::Left:  newX -= step; break;
        case Direction::Right: newX += step; break;
    }

    if (!canMove(playerId, newX, newY)) return false;

    p.x = newX;
    p.y = newY;

    int gridX = static_cast<int>(p.x) / 50;
    int gridY = static_cast<int>(p.y) / 50;
    Case& cell = board.getCase(gridX, gridY);

    if (p.getRole() == PlayerRole::PacMan && cell.hasPacGomme()) {
        p.eat(true, nullptr);
        cell.removePacGomme();
    } else if (p.getRole() == PlayerRole::Ghost) {
        for (auto& [otherId, otherPtr] : players) {
            Player& other = *otherPtr;
            if (other.getRole() == PlayerRole::PacMan) {
                int otherX = static_cast<int>(other.x) / 50;
                int otherY = static_cast<int>(other.y) / 50;
                if (otherX == gridX && otherY == gridY) {
                    p.eat(false, &other);
                }
            }
        }
    }

    return true;
}




Player& Game::getPlayerInfo(uint32_t playerId) {
    auto it = players.find(playerId);
    if (it == players.end()) {
        throw std::runtime_error("Player not found");
    }
    return *it->second;
}



void Game::addPlayer(uint32_t id, float x, float y, PlayerRole role = PlayerRole::Spectator) {
    auto player = std::make_unique<Player>(id, role);
    player->x = x;
    player->y = y;

    players.emplace(id, std::move(player));
}



void Game::startChrono() {
    std::lock_guard<std::mutex> lock(chronoMutex);
    chronoStart = std::chrono::steady_clock::now();
}

void Game::resetChrono() {
    std::lock_guard<std::mutex> lock(chronoMutex);
    chronoStart = std::chrono::steady_clock::now();
}


void Game::startGameLoop(int tickMs_, InputQueue& inputQueue, ServerNetwork& server) {
    tickMs = tickMs_;
    running.store(true);
    gameStarted.store(false);
    preGameElapsed = 0.0;
    gameElapsed = 0.0;

    preGameStart = std::chrono::steady_clock::now();

    gameThread = std::thread([this, &inputQueue, &server]() {
        while (running.load()) {
            processInputs(inputQueue);
            if(room){
                room->broadcastState();
            }
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = now - preGameStart;

            preGameElapsed = elapsed.count();

            if (!gameStarted.load()) {
                if (preGameElapsed >= preGameDelay) {
                    gameStarted.store(true);
                    chronoStart = now;
                    gf::Log::info("Début de la partie !\n");
                }
            } else {
                // --- phase de jeu actif ---
                std::chrono::duration<double> gameTime = now - chronoStart;
                gameElapsed = gameTime.count();
                if (gameElapsed >= T_GAME) {
                    gf::Log::info("Partie terminée !\n");
                    running.store(false);
                    gameStarted.store(false);
                    break;
                }

                // --- logique ---
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(tickMs));
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
        // spawn Fantome
        p.x = (board.getWidth() / 2) * 50.0f;
        p.y = (board.getHeight() / 2) * 50.0f;
    } else if (p.getRole() == PlayerRole::PacMan) {
        // spawn PacMan
        p.x = 50.0f;
        p.y = 50.0f;
    } else {
        // Spectator spawn
        p.x = 30.0f;
        p.y = 30.0f;
    }
}

void Game::processInputs(InputQueue& queue) {
    while (auto inputOpt = queue.pop()) {
        auto& input = *inputOpt;
        bool moved = requestMove(input.playerId, input.dir);
        if (moved) {
            auto& p = getPlayerInfo(input.playerId);
            gf::Log::info("Player %u moved to (%.1f, %.1f)", p.id, p.x, p.y);
        }
    }
}

double Game::getPreGameElapsed() const {
    std::lock_guard<std::mutex> lock(chronoMutex);
    return preGameElapsed;
}

double Game::getElapsedSeconds() const {
    std::lock_guard<std::mutex> lock(chronoMutex);
    return gameElapsed;
}

bool Game::isPreGame() const {
    std::lock_guard<std::mutex> lock(chronoMutex);
    return !gameStarted.load() && preGameElapsed < preGameDelay;
}

bool Game::isGameOver() const {
    std::lock_guard<std::mutex> lock(chronoMutex);
    return gameStarted.load() && gameElapsed >= T_GAME;
}


