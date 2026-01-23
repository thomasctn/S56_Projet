#include "Game.h"
#include "ServerNetwork.h"
#include "Room.h"
#include "../common/Constants.h"
#include "BotManager.h"


Game::Game(int width, int height) : board(width, height) {
    board.placeRandomPacGommes();
    srand(static_cast<unsigned int>(time(nullptr)));
}


bool Game::canMove(uint32_t playerId, float newX, float newY) const {
    const auto& boardRef = getBoard();

    int gridX = static_cast<int>(newX) / 50;
    int gridY = static_cast<int>(newY) / 50;

    if (!boardRef.isInside(gridX, gridY)) return false;

    const auto& cell = boardRef.getCase(gridX, gridY);

    // Vérifie la walkabilité générale
    if (!cell.isWalkable()) return false;

    // Règle spéciale PacMan : interdit HUT
    auto itCurrent = players.find(playerId);
    if (itCurrent == players.end()) return false;
    const Player& currentPlayer = *itCurrent->second;

    if (currentPlayer.getRole() == PlayerRole::PacMan &&
        cell.getType() == CellType::Hut) {
        return false;
    }

    // Collision avec les autres joueurs
    for (const auto& [id, playerPtr] : players) {
        if (id == playerId) continue;

        const Player& other = *playerPtr;

        int px = static_cast<int>(other.x) / 50;
        int py = static_cast<int>(other.y) / 50;

        if (px == gridX && py == gridY) {
            // PacMan et Ghost peuvent marcher l'un sur l'autre
            if ((currentPlayer.getRole() == PlayerRole::Ghost && other.getRole() == PlayerRole::PacMan) ||
                (currentPlayer.getRole() == PlayerRole::PacMan && other.getRole() == PlayerRole::Ghost)) {
                continue; // autorisé
            }

            // Ghost sur Ghost : autorisé
            if (currentPlayer.getRole() == PlayerRole::Ghost && other.getRole() == PlayerRole::Ghost) {
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

    int X = static_cast<int>(p.x) / 50;
    int Y = static_cast<int>(p.y) / 50;

    if (board.isHole(X, Y))
    {
        Position target = board.getLinkedHole(X, Y);
        p.x = target.x * step;
        p.y = target.y * step;
    }


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

    if (p.getRole() == PlayerRole::PacMan && board.hasPacgomme(gridX, gridY)) {
        PacGommeType type = board.getPacGommeType(gridX, gridY);
        p.eat(type, nullptr); // type de pac-gomme
        board.removePacgomme(gridX, gridY);
    } else if (p.getRole() == PlayerRole::Ghost) {
        for (auto& [otherId, otherPtr] : players) {
            Player& other = *otherPtr;
            if (other.getRole() == PlayerRole::PacMan) {
                int otherX = static_cast<int>(other.x) / 50;
                int otherY = static_cast<int>(other.y) / 50;
                if (otherX == gridX && otherY == gridY) {
                    p.eat(std::nullopt, &other); // pas de pac-gomme, interaction joueur
                }
            }
        }
    }



    // --- Ajout de trace ---
    if (botManager) {
        int nodeX = static_cast<int>(p.x / 50);
        int nodeY = static_cast<int>(p.y / 50);

        Node* node = botManager->getNode(nodeX, nodeY);
        if (node) {
            Trace t;
            t.ownerId   = playerId;
            t.type      = (p.getRole() == PlayerRole::PacMan) ? TraceType::PacMan : TraceType::Ghost;
            t.intensity = 1.0f;

            botManager->getTraces().add(node, t);
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

void Game::removePlayer(uint32_t playerId) {
    players.erase(playerId);
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
        unsigned int lastRemaining = preGameDelay + 1;

        static auto lastUpdate = std::chrono::steady_clock::now();

        while (running.load()) {
            auto now = std::chrono::steady_clock::now();

            // --- calcul du delta time depuis le dernier tick ---
            auto nowUpdate = std::chrono::steady_clock::now();
            std::chrono::duration<double> dt = nowUpdate - lastUpdate;
            lastUpdate = nowUpdate;

            // --- mise à jour des traces ---
            if (botManager) {
                botManager->updateTraces();
                // --- update des bots ---
                botManager->update(dt.count());
            }

            // --- traitement des inputs des joueurs humains ---
            processInputs(inputQueue);

            if (room) {
                room->broadcastState();
            }

            std::chrono::duration<double> elapsed = now - preGameStart;
            preGameElapsed = elapsed.count();

            if (!gameStarted.load()) {
                // --- phase pré-jeu ---
                unsigned int remaining = 0;
                if (preGameElapsed < preGameDelay) {
                    remaining = static_cast<unsigned int>(
                        std::ceil(preGameDelay - preGameElapsed)
                    );
                }

                if (room && remaining != lastRemaining) {
                    room->broadcastPreGame(remaining);
                    lastRemaining = remaining;
                }

                if (preGameElapsed >= preGameDelay) {
                    gameStarted.store(true);
                    chronoStart = now;
                    gf::Log::info("Début de la partie !\n");
                }
            } else {
                // --- phase de jeu actif ---
                std::chrono::duration<double> gameTime = now - chronoStart;
                gameElapsed = gameTime.count();
                unsigned int nbPacGommes = board.getPacgommeCount();
                if (room && (gameElapsed >= room->getGameDuration() || nbPacGommes == 0)) {
                    gf::Log::info("Partie terminée !\n");
                    running.store(false);
                    gameStarted.store(false);
                    GameEndReason reason = (gameElapsed >= room->getGameDuration())
                                              ? GameEndReason::TIME_OUT
                                              : GameEndReason::ALL_DOT_EATEN;
                    room->endGame(reason);
                    break;
                }
            }

            // --- update des mouvements des joueurs humains ---
            if (gameStarted.load()) {
                updateMovement(dt.count());
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(tickMs));
        }
    });
}


void Game::stopGameLoop() {
    running = false;

    if (gameThread.joinable() && std::this_thread::get_id() != gameThread.get_id())
        gameThread.join();
}


void Game::spawnPlayer(Player& p) {
    if (p.getRole() == PlayerRole::Ghost) {
        int centerX = board.getWidth() / 2;
        int centerY = board.getHeight() / 2;

        int offsetX = rand() % 3 - 1;
        int offsetY = rand() % 3 - 1;

        p.x = (centerX + offsetX) * 50.0f;
        p.y = (centerY + offsetY) * 50.0f;
    } else if (p.getRole() == PlayerRole::PacMan) {
        int w = board.getWidth();
        int h = board.getHeight();

        int corner = rand() % 4;
        switch(corner) {
            case 0:
                p.x = 1*50.0f;
                p.y = 1*50.0f;
                break;
            case 1:
                p.x = 1*50.0f;
                p.y = (h-2)*50.0f;
                break;
            case 2:
                p.x = (w-2)*50.0f;
                p.y = 1*50.0f;
                break;
            case 3:
                p.x = (w-2)*50.0f;
                p.y = (h-2)*50.0f;
                break;
        }
    } else {
        p.x = 2*50.0f;
        p.y = 2*50.0f;
    }
}


void Game::processInputs(InputQueue& queue) {
    while (auto inputOpt = queue.pop()) {
        auto& input = *inputOpt;

        auto it = players.find(input.playerId);
        if (it == players.end()) continue;

        Player& p = *it->second;
        p.bufferedDir = input.dir;
        p.hasMoveRequest = true;
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
    return gameStarted.load() && gameElapsed >= room->getGameDuration();
}

void Game::updateMovement(double dt) {
    constexpr float step = 50.0f;

    for (auto& [id, playerPtr] : players) {
        Player& p = *playerPtr;

        p.update(dt);

        if (!p.hasMoveRequest)
            continue;

        p.moveAccumulator += dt;

        double interval = 1.0 / p.moveRate;
        if (p.moveAccumulator < interval)
            continue;

        p.moveAccumulator -= interval;

        requestMove(id, p.bufferedDir);

        p.hasMoveRequest = false;
    }
}
