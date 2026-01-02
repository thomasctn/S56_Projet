#include "GameNetworkServer.h"
#include <gf/Log.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Event.h>
#include <gf/Shapes.h>
#include <thread>
#include <chrono>

int main() {
    gf::Log::info("Démarrage du serveur...");

    GameNetworkServer server;

    // 🔁 Thread réseau
    std::thread serverThread([&server]() {
        server.run();
    });

    // 🪟 Fenêtre serveur
    const int windowWidth = 800;
    const int windowHeight = 600;
    gf::Window mainWindow("SERVER VIEW", {windowWidth, windowHeight});
    gf::RenderWindow window(mainWindow);

    const float RENDER_SIZE = 500.0f; // Taille max du plateau à l'écran

    while (mainWindow.isOpen()) {
        gf::Event event;
        while (mainWindow.pollEvent(event)) {
            if (event.type == gf::EventType::Closed) {
                mainWindow.close();
            }
        }

        window.clear(gf::Color::Black);

        {
            // 🔒 Accès thread-safe
            std::lock_guard<std::mutex> lock(server.getClientsMutex());

            // 🟩 Plateau
            auto& plateau = server.getGame().getPlateau();

            int mapWidth  = plateau.getWidth();
            int mapHeight = plateau.getHeight();

            // Calcul du tileSize auto pour que le plateau rentre dans RENDER_SIZE
            float tileSizeX = RENDER_SIZE / mapWidth;
            float tileSizeY = RENDER_SIZE / mapHeight;
            float tileSize = std::min(tileSizeX, tileSizeY);

            // Calcul de l'offset pour centrer le plateau dans la fenêtre
            float offsetX = (windowWidth  - tileSize * mapWidth) / 2.0f;
            float offsetY = (windowHeight - tileSize * mapHeight) / 2.0f;

            // Rendu des cases
            for (int y = 0; y < mapHeight; ++y) {
                for (int x = 0; x < mapWidth; ++x) {
                    const Case& cell = plateau.getCase(x, y);

                    gf::RectangleShape tile({tileSize, tileSize});
                    tile.setPosition({x * tileSize + offsetX, y * tileSize + offsetY});

                    switch (cell.getType()) {
                        case CellType::Wall:
                            tile.setColor(gf::Color::White);
                            window.draw(tile);
                            break;
                        case CellType::Hut:
                            tile.setColor(gf::Color::Red);
                            window.draw(tile);
                            break;
                        case CellType::Floor:
                        default:
                            // sol → ne rien dessiner ou gris léger
                            break;
                    }
                }
            }

            // 🟦 Rendu des joueurs
            for (auto& c : server.getClients()) {
                gf::RectangleShape player({tileSize, tileSize});
                // c.state.x / y sont en pixels, donc on convertit en coordonnées plateau
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

    serverThread.join();
    return 0;
}

