#include "ServerNetwork.h"
#include "../common/Constants.h"
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
    gRunning = false;          // signal pour la boucle principale
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

        // Choisir la Room à afficher
        RoomId displayRoomId = 1;

        while (mainWindow.isOpen() && gRunning) {
            gf::Event event;
            while (mainWindow.pollEvent(event)) {
                if (event.type == gf::EventType::Closed) {
                    gRunning = false;
                    server.stop();
                }
            }

            window.clear(gf::Color::Black);

            // --- Récupération de la Room et du Game ---
            try {
                Room& room = server.getLobby().getRoom(displayRoomId);
                Game& game = room.getGameRef();
                auto& board = game.getBoard();
                auto& players = game.getPlayers();

                int mapWidth  = board.getWidth();
                int mapHeight = board.getHeight();

                float tileSize = std::min(RENDER_SIZE / mapWidth, RENDER_SIZE / mapHeight);
                float offsetX = (windowWidth  - tileSize * mapWidth) / 2.0f;
                float offsetY = (windowHeight - tileSize * mapHeight) / 2.0f;

                // --- Dessiner le plateau ---
                for (int y = 0; y < mapHeight; ++y) {
                    for (int x = 0; x < mapWidth; ++x) {
                        const Case& cell = board.getCase(x, y);
                        gf::RectangleShape tile({tileSize, tileSize});
                        tile.setPosition({x * tileSize + offsetX, y * tileSize + offsetY});

                        switch (cell.getType()) {
                            case CellType::Wall:  tile.setColor(gf::Color::White); break;
                            case CellType::Hut:   tile.setColor(gf::Color::Red); break;
                            case CellType::Floor: tile.setColor(gf::Color::fromRgb(0.3f, 0.3f, 0.3f)); break;
                            default: continue;
                        }
                        window.draw(tile);

                        if (cell.hasPacGomme()) {
                            gf::CircleShape pacGommeShape(tileSize / 6.0f); 
                            pacGommeShape.setOrigin({tileSize/12.0f, tileSize/12.0f});
                            pacGommeShape.setPosition({x * tileSize + offsetX + tileSize/2,
                                                       y * tileSize + offsetY + tileSize/2});
                            pacGommeShape.setColor(gf::Color::Yellow);
                            window.draw(pacGommeShape);
                        }
                    }
                }

                // --- Dessiner les joueurs ---
                for (auto& [id, playerPtr] : players) {
                    if (!playerPtr) continue;
                    Player& p = *playerPtr;

                    gf::RectangleShape playerRect({tileSize, tileSize});
                    float px = p.x / 50.0f * tileSize + offsetX;
                    float py = p.y / 50.0f * tileSize + offsetY;
                    playerRect.setPosition({px, py});

                    switch (p.getRole()) {
                        case PlayerRole::PacMan:    playerRect.setColor(gf::Color::Yellow); break;
                        case PlayerRole::Ghost:     playerRect.setColor(gf::Color::Violet); break;
                        case PlayerRole::Spectator: playerRect.setColor(gf::Color::Cyan); break;
                    }
                    window.draw(playerRect);

                    static gf::Font font("../common/fonts/arial.ttf");
                    gf::Text scoreText;
                    scoreText.setFont(font);
                    scoreText.setCharacterSize(16);
                    scoreText.setColor(gf::Color::White);
                    scoreText.setString(std::to_string(p.score));
                    scoreText.setPosition({px + 5, py - 18});
                    window.draw(scoreText);
                }

                // --- Affichage du chrono ---
                std::string chronoMessage;
                int remainingTime = 0;

                if (game.isPreGame()) {
                    remainingTime = PRE_GAME_DELAY - static_cast<int>(game.getPreGameElapsed());
                    if (remainingTime < 0) remainingTime = 0;
                    chronoMessage = "Début de la partie dans : " + std::to_string(remainingTime) + "s";
                } else if (game.isGameStarted()) {
                    remainingTime = T_GAME - static_cast<int>(game.getElapsedSeconds());
                    if (remainingTime < 0) remainingTime = 0;
                    chronoMessage = "Temps restant : " + std::to_string(remainingTime) + "s";
                } else if (game.isGameOver()) {
                    chronoMessage = "Partie terminée !";
                }

                static gf::Font font("../common/fonts/arial.ttf");
                gf::Text chronoText;
                chronoText.setFont(font);
                chronoText.setCharacterSize(24);
                chronoText.setColor(gf::Color::White);
                chronoText.setString(chronoMessage);
                chronoText.setPosition({12.0f, 20.0f});
                window.draw(chronoText);

            } catch (const std::out_of_range& e) {
                gf::Log::error("Room %d introuvable !\n", displayRoomId);
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
