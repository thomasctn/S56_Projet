#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Shapes.h>
#include <gf/Color.h>
#include <gf/Log.h>
#include <thread>
#include <chrono>
#include <vector>
#include <gf/Keyboard.h>
#include <gf/Event.h>
#include <mutex>
#include "../common/Types.h"

int main() {
    gf::Log::info("Démarrage du client...\n");

    gf::TcpSocket socket("127.0.0.1", "5000");
    if (!socket) {
        gf::Log::error("Impossible de se connecter au serveur !\n");
        return -1;
    }

    gf::Window mainWindow("GF Sync Boxes", {800, 600});
    gf::RenderWindow window(mainWindow);

    char currentDir = 0;
    char lastSentDir = 0;
    auto lastSendTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds repeatDelay(200); // 0,5s

    std::vector<ClientState> states;
    std::mutex statesMutex;
    bool running = true;
    socket.setNonBlocking();

    std::thread receiver([&socket, &mainWindow, &states, &statesMutex]() {
        uint8_t buffer[1024];

        while (mainWindow.isOpen()) {
            gf::SocketDataResult result = socket.recvRawBytes({buffer, sizeof(buffer)});

            if (result.status == gf::SocketStatus::Data) {
                std::lock_guard<std::mutex> lock(statesMutex);
                states.clear();
                size_t offset = 0;
                while (offset + sizeof(ClientState) <= result.length) {
                    states.push_back(deserializeClientState(buffer, offset));
                }
            } else if (result.status == gf::SocketStatus::Block) {
                // Pas de donnée disponible, on continue la boucle
            } else if (result.status == gf::SocketStatus::Close) {
                gf::Log::info("Serveur déconnecté\n");
                mainWindow.close();
                break;
            } else if (result.status == gf::SocketStatus::Error) {
                gf::Log::error("Erreur réseau côté client\n");
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });


    while (running && mainWindow.isOpen()) {
        gf::Event event;

        // Gestion des événements
        while (mainWindow.pollEvent(event)) {
            if (event.type == gf::EventType::Closed) {
                gf::Log::info("Fermeture demandée par l'utilisateur\n");
                running = false;
                mainWindow.close();
            } else if (event.type == gf::EventType::KeyPressed) {
                switch (event.key.keycode) {
                    case gf::Keycode::Up:    currentDir = 'U'; break;
                    case gf::Keycode::Down:  currentDir = 'D'; break;
                    case gf::Keycode::Left:  currentDir = 'L'; break;
                    case gf::Keycode::Right: currentDir = 'R'; break;
                    default: currentDir = 0; break;
                }
            } else if (event.type == gf::EventType::KeyReleased) {
                if ((event.key.keycode == gf::Keycode::Up && currentDir == 'U') ||
                    (event.key.keycode == gf::Keycode::Down && currentDir == 'D') ||
                    (event.key.keycode == gf::Keycode::Left && currentDir == 'L') ||
                    (event.key.keycode == gf::Keycode::Right && currentDir == 'R')) {
                    currentDir = 0;
                }
            }
        }

        // Envoi répété si touche maintenue
        auto now = std::chrono::steady_clock::now();
        if (currentDir != 0 && (currentDir != lastSentDir || now - lastSendTime >= repeatDelay)) {
            socket.sendRawBytes({reinterpret_cast<uint8_t*>(&currentDir), 1});
            lastSentDir = currentDir;
            lastSendTime = now;
            gf::Log::info("Envoi : touche '%c'\n", currentDir);
        }

        // Rendu
        window.clear(gf::Color::Black);
        {
            std::lock_guard<std::mutex> lock(statesMutex);
            for (auto& s : states) {
                gf::RectangleShape box({50.0f, 50.0f});
                box.setPosition({s.x, s.y});
                box.setColor({
                    static_cast<uint8_t>((s.color >> 24) & 0xFF),
                    static_cast<uint8_t>((s.color >> 16) & 0xFF),
                    static_cast<uint8_t>((s.color >> 8) & 0xFF),
                    static_cast<uint8_t>(s.color & 0xFF)
                });
                window.draw(box);
            }
        }
        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    if (receiver.joinable()) {
        receiver.join();
    }

    gf::Log::info("Client fermé proprement\n");
    return 0;
}
