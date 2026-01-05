#include "GameNetworkServer.h"
#include <gf/Log.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Event.h>
#include <gf/Shapes.h>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>


// Savoir si on dev. A DESACTIVER en cas PUBLICATION sur un SERVER
#define DEV true

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
                std::lock_guard<std::mutex> lock(server.getClientsMutex());

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

                // Joueurs
                for (auto& c : server.getClients()) {
                    gf::RectangleShape player({tileSize, tileSize});
                    float px = c.state.x / 50.0f * tileSize + offsetX;
                    float py = c.state.y / 50.0f * tileSize + offsetY;
                    player.setPosition({px, py});
                    player.setColor(gf::Color::Green);
                    window.draw(player);
                }
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
