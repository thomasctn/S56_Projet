#include "GameNetworkServer.h"
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


// Savoir si on dev. A DESACTIVER en cas PUBLICATION sur un SERVER
#define DEV true
#define T_GAME 300

GameNetworkServer* gServer = nullptr;
std::atomic<bool> gRunning{true}; // drapeau global pour ctrl+c

void sigintHandler(int) {
    gRunning = false;          // signal pour la boucle principale
    if (gServer) gServer->stop();
}

int main() {
    gf::Log::info("Démarrage du serveur...");

    GameNetworkServer server;
    gServer = &server;
    std::signal(SIGINT, sigintHandler);  // ctrl+c stoppe le serveur et déclenche gRunning = false

    // Thread réseau
    std::thread serverThread([&server]() {
        server.run();
    });


    // Fenêtre serveur
    if (DEV){
        const int windowWidth = 800;
        const int windowHeight = 600;
        gf::Window mainWindow("SERVER VIEW", {windowWidth, windowHeight});
        gf::RenderWindow window(mainWindow);

        const float RENDER_SIZE = 500.0f;

        while (mainWindow.isOpen() && gRunning) {
            gf::Event event;
            while (mainWindow.pollEvent(event)) {
                if (event.type == gf::EventType::Closed) {
                    gRunning = false;  // arrête la boucle
                    server.stop();     // stop serveur
                }
            }

            window.clear(gf::Color::Black);

            {
                std::lock_guard<std::mutex> lock(server.getPlayersMutex());

                auto& plateau = server.getGame().getPlateau();
                int mapWidth  = plateau.getWidth();
                int mapHeight = plateau.getHeight();

                float tileSize = std::min(RENDER_SIZE / mapWidth, RENDER_SIZE / mapHeight);
                float offsetX = (windowWidth  - tileSize * mapWidth) / 2.0f;
                float offsetY = (windowHeight - tileSize * mapHeight) / 2.0f;

                // Cases
                for (int y = 0; y < mapHeight; ++y) {
                    for (int x = 0; x < mapWidth; ++x) {
                        const Case& cell = plateau.getCase(x, y);
                        gf::RectangleShape tile({tileSize, tileSize});
                        tile.setPosition({x * tileSize + offsetX, y * tileSize + offsetY});

                        switch (cell.getType()) {
                            case CellType::Wall: tile.setColor(gf::Color::White); break;
                            case CellType::Hut:  tile.setColor(gf::Color::Red); break;
                            case CellType::Floor:
                            default: continue;
                        }
                        window.draw(tile);
                    }
                }

                // --- Joueurs ---
                for (auto& [id, playerPtr] : server.getGame().getPlayers()) {
                    if (!playerPtr) continue;
                    Player& p = *playerPtr;

                    gf::RectangleShape playerRect({tileSize, tileSize});
                    float px = p.x / 50.0f * tileSize + offsetX;
                    float py = p.y / 50.0f * tileSize + offsetY;
                    playerRect.setPosition({px, py});

                    // couleur selon le rôle
                    switch (p.getRole()) {
                        case PlayerRole::PacMan:    playerRect.setColor(gf::Color::Yellow); break;
                        case PlayerRole::Ghost:     playerRect.setColor(gf::Color::Violet); break; // violet
                        case PlayerRole::Spectator: playerRect.setColor(gf::Color::Cyan); break; // gris
                    }

                    window.draw(playerRect);

                    // Affichage du score au-dessus du joueur
                    static gf::Font font("../common/fonts/arial.ttf"); // une seule fois
                    gf::Text scoreText;
                    scoreText.setFont(font);
                    scoreText.setCharacterSize(16);
                    scoreText.setColor(gf::Color::White);
                    scoreText.setString(std::to_string(p.score));
                    scoreText.setPosition({px, py - 18}); // juste au-dessus du joueur
                    window.draw(scoreText);
                }



                // --- Affichage du chrono ---
                double elapsed = server.getGame().getElapsedSeconds();
                int seconds = static_cast<int>(elapsed);

                static gf::Font font("../common/fonts/arial.ttf");
                gf::Text chronoText;
                chronoText.setFont(font);
                chronoText.setCharacterSize(24); // taille texte
                chronoText.setColor(gf::Color::White);
                chronoText.setString("Temps restant : " + std::to_string(T_GAME - seconds) + "s");
                chronoText.setPosition({12.0f, 20.0f}); // coin haut gauche
                window.draw(chronoText);
            }

            window.display();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        // fermer la fenêtre si ctrl+c
        if (mainWindow.isOpen()) mainWindow.close();
    }
    serverThread.join();
    gf::Log::info("Serveur fermé proprement\n");
    return 0;
}
