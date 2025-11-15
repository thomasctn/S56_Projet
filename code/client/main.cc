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
    std::vector<ClientState> states;
    std::mutex statesMutex;
    bool running = true;
    socket.setNonBlocking();

    // ID local du client
    uint32_t myId = 0;

    // fonction pour générer une couleur basée sur l'ID
    auto colorFromId = [](uint32_t id) -> gf::Color4f {
        float r = float((id * 50) % 256) / 255.0f;
        float g = float((id * 80) % 256) / 255.0f;
        float b = float((id * 110) % 256) / 255.0f;
        return gf::Color4f(r, g, b, 1.0f);
    };

    std::thread receiver([&]() {
        uint8_t buffer[1024];
        while (running && mainWindow.isOpen()) {
            gf::SocketDataResult result = socket.recvRawBytes({buffer, sizeof(buffer)});

            if (result.status == gf::SocketStatus::Data) {
                std::lock_guard<std::mutex> lock(statesMutex);
                states.clear();
                size_t offset = 0;
                while (offset + sizeof(ClientState) <= result.length) {
                    ClientState s = deserializeClientState(buffer, offset);
                    states.push_back(s);
                    if (s.id == myId && myId == 0) {
                        myId = s.id; // récupérer l'ID assigné par le serveur
                    }
                }
            } else if (result.status == gf::SocketStatus::Block) {
                // pas de données disponibles, continue
            } else if (result.status == gf::SocketStatus::Close) {
                gf::Log::info("Serveur déconnecté\n");
                running = false;
                mainWindow.close();
                break;
            } else if (result.status == gf::SocketStatus::Error) {
                gf::Log::error("Erreur réseau côté client\n");
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    auto lastSend = std::chrono::steady_clock::now();

    int map_size =27;
    std::vector<std::vector<int>> map(map_size, std::vector<int>(map_size, 0)); //provisoire : 0=floor 1=wall 2=hut

    map[0][0]=1;
    map[1][0]=1;
    map[2][0]=1;
    map[3][0]=1;
    map[4][0]=1;
    map[5][0]=1;
    map[6][0]=1;
    map[7][0]=1;
    map[8][0]=1;
    map[9][0]=1;
    map[10][0]=1;
    map[11][0]=1;
    map[12][0]=1;
    map[13][0]=1;

    map[0][1]=1;
    map[0][2]=1;
    map[0][3]=1;
    map[0][4]=1;
    map[0][5]=1;
    map[0][6]=1;
    map[0][7]=1;

    map[2][3]=2;



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
                currentDir = 0;
            }
        }



        // envoyer la touche avec un délai si maintenue
        if (currentDir != 0) {
            auto now = std::chrono::steady_clock::now();
            if (now - lastSend > std::chrono::milliseconds(100)) {
                gf::Log::info("Envoi : touche '%c'\n", currentDir);
                socket.sendRawBytes({reinterpret_cast<uint8_t*>(&currentDir), 1});
                lastSend = now;
            }
        }

        // Rendu
        window.clear(gf::Color::Black);
        {
            std::lock_guard<std::mutex> lock(statesMutex);
            for (auto& s : states) {
                gf::RectangleShape box({50.0f, 50.0f});
                box.setPosition({s.x, s.y});
                gf::Color4f c = (s.id == myId) ? gf::Color4f(1.0f, 0.0f, 0.0f, 1.0f) : colorFromId(s.id);
                box.setColor(c);
                window.draw(box);
            }

            for(int y =0; y<map_size;y++){
                for(int x=0;x<map_size;x++){
                    if(map[y][x]==1){
                        gf::RectangleShape wall({50.0f,50.0f});
                        wall.setPosition({float(x*50),float(y*50)});
                        wall.setColor(gf::Color::White);
                        window.draw(wall);

                    }

                    if(map[y][x]==2){
                        gf::RectangleShape wall({50.0f,50.0f});
                        wall.setPosition({float(x*50),float(y*50)});
                        wall.setColor(gf::Color::Red);
                        window.draw(wall);

                    }
                }
            }
        }
        window.display();

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    if (receiver.joinable()) {
        receiver.join();
    }

    gf::Log::info("Client fermé proprement\n");
    return 0;
}
