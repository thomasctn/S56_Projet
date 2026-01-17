#include "ServerNetwork.h"
#include "../common/Constants.h"
#include "../common/Protocol.h"

#include "Lobby.h"
#include "Room.h"
#include "Game.h"

#include <gf/Log.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Event.h>
#include <gf/Shapes.h>
#include <gf/Text.h>
#include <gf/Font.h>

#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>

ServerNetwork* gServer = nullptr;
std::atomic<bool> gRunning{true}; // drapeau global pour ctrl+c

void sigintHandler(int) {
    gRunning = false;
    if (gServer) gServer->stop();
}

int main() {
    gf::Log::info("Démarrage du serveur...\n");

    ServerNetwork server;
    gServer = &server;
    std::signal(SIGINT, sigintHandler);  // ctrl+c stoppe le serveur

    // --- Thread réseau ---
    std::thread serverThread([&server]() {
        server.run();
    });

    // --- Fenêtre serveur (dev mode) ---
    if (DEV) {
        const int windowWidth = 800;
        const int windowHeight = 600;
        gf::Window mainWindow("SERVER VIEW", {windowWidth, windowHeight});
        gf::RenderWindow window(mainWindow);

        const float RENDER_SIZE = 500.0f;
        RoomId displayRoomId = 1;

        static gf::Font font("../common/fonts/arial.ttf");



        bool renderRunning = true;

        while (mainWindow.isOpen() && gRunning && renderRunning) {
            // --- Gestion events ---
            gf::Event event;
            while (mainWindow.pollEvent(event)) {
                if (event.type == gf::EventType::Closed) {
                    gRunning = false;
                    server.stop();
                }
            }

            window.clear(gf::Color::Black);

            Room* roomPtr = nullptr;
            Game* game = nullptr;

            try {
                roomPtr = &server.getLobby().getRoom(displayRoomId);
                if (roomPtr) game = &roomPtr->getGame();
            } catch (...) {
                gf::Log::error("Room/Game introuvable !");
                renderRunning = false;
            }

            if (game) {
                // Vérifier si la partie est terminée
                if (!game->isPreGame() && !game->isGameStarted()) {
                    renderRunning = false; // sortir proprement
                    continue;
                }

                const Board& board = game->getBoard();
                int mapWidth  = board.getWidth();
                int mapHeight = board.getHeight();
                float tileSize = std::min(RENDER_SIZE / mapWidth, RENDER_SIZE / mapHeight);
                float offsetX = (windowWidth  - tileSize * mapWidth) / 2.0f;
                float offsetY = (windowHeight - tileSize * mapHeight) / 2.0f;

                // Dessin plateau
                for (int y = 0; y < mapHeight; ++y) {
                    for (int x = 0; x < mapWidth; ++x) {
                        const Case* cell = &board.getCase(x, y);
                        if (!cell) continue;

                        gf::RectangleShape tile({tileSize, tileSize});
                        tile.setPosition({x * tileSize + offsetX, y * tileSize + offsetY});

                        switch (cell->getType()) {
                            case CellType::Wall:  tile.setColor(gf::Color::White); break;
                            case CellType::Hut:   tile.setColor(gf::Color::Red); break;
                            case CellType::Floor: tile.setColor(gf::Color::fromRgb(0.3f, 0.3f, 0.3f)); break;
                            default: continue;
                        }
                        window.draw(tile);

                        if (board.hasPacgomme(x,y)) {
                            gf::CircleShape pacGommeShape(tileSize / 6.0f);
                            pacGommeShape.setOrigin({tileSize/12.0f, tileSize/12.0f});
                            pacGommeShape.setPosition({x * tileSize + offsetX + tileSize/2,
                                                      y * tileSize + offsetY + tileSize/2});
                            pacGommeShape.setColor(gf::Color::Yellow);
                            window.draw(pacGommeShape);
                        }
                    }
                }

                // --- Dessin des traces ---
                if (roomPtr && roomPtr->getBotManager()) {
                    auto& traceMap = roomPtr->getBotManager()->getTraces();
                    for (auto& [node, traces] : traceMap.getAll()) {
                        for (const Trace& t : traces) {
                            gf::RectangleShape traceRect({tileSize, tileSize});
                            traceRect.setPosition({node->x * tileSize + offsetX, node->y * tileSize + offsetY});

                            if (t.type == TraceType::PacMan) {
                                traceRect.setColor(gf::Color::fromRgba32(
                                    static_cast<uint8_t>(255),                      // R
                                    static_cast<uint8_t>(0),                        // G
                                    static_cast<uint8_t>(0),                        // B
                                    static_cast<uint8_t>(t.intensity * 0.5f * 255) // A
                                ));

                            } else {
                                traceRect.setColor(gf::Color::fromRgba32(
                                    static_cast<uint8_t>(0),
                                    static_cast<uint8_t>(0),
                                    static_cast<uint8_t>(255),
                                    static_cast<uint8_t>(t.intensity * 0.5f * 255)
                                ));

                            }

                            window.draw(traceRect);
                        }
                    }
                }

                // Joueurs
                struct PlayerSnapshot { float x,y; PlayerRole role; int score; };
                std::vector<PlayerSnapshot> players;
                for (auto& [id, pPtr] : game->getPlayers()) {
                    if (!pPtr) continue;
                    players.push_back({pPtr->x, pPtr->y, pPtr->getRole(), pPtr->score});
                }

                // Dessin joueurs
                for (auto& p : players) {
                    gf::RectangleShape playerRect({tileSize, tileSize});
                    playerRect.setPosition({p.x / 50.f * tileSize + offsetX, p.y / 50.f * tileSize + offsetY});
                    switch (p.role) {
                        case PlayerRole::PacMan:    playerRect.setColor(gf::Color::Yellow); break;
                        case PlayerRole::Ghost:     playerRect.setColor(gf::Color::Violet); break;
                        case PlayerRole::Spectator: playerRect.setColor(gf::Color::Cyan); break;
                    }
                    window.draw(playerRect);

                    gf::Text scoreText;
                    scoreText.setFont(font);
                    scoreText.setCharacterSize(16);
                    scoreText.setColor(gf::Color::White);
                    scoreText.setString(std::to_string(p.score));
                    scoreText.setPosition({p.x / 50.f * tileSize + offsetX + 5, p.y / 50.f * tileSize + offsetY - 18});
                    window.draw(scoreText);
                }

                // Chrono
                std::string chronoMessage;
                if (game->isPreGame()) {
                    int remainingTime = PRE_GAME_DELAY - static_cast<int>(game->getPreGameElapsed());
                    chronoMessage = "Début dans : " + std::to_string(std::max(0, remainingTime)) + "s";
                } else if (game->isGameStarted()) {
                    int remainingTime = roomPtr->getGameDuration() - static_cast<int>(game->getElapsedSeconds());
                    chronoMessage = "Temps restant : " + std::to_string(std::max(0, remainingTime)) + "s";
                } else {
                    chronoMessage = "Partie terminée !";
                }

                gf::Text chronoText;
                chronoText.setFont(font);
                chronoText.setCharacterSize(24);
                chronoText.setColor(gf::Color::White);
                chronoText.setString(chronoMessage);
                chronoText.setPosition({12.f, 20.f});
                window.draw(chronoText);
            }

            window.display();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        if (mainWindow.isOpen()) mainWindow.close();
    }

    serverThread.join();
    gf::Log::info("Serveur fermé proprement\n");
    return 0;
}
