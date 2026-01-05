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
#include "Renderer.h"
#include "Structures.h"


Renderer renderer;

int main()
{
    gf::Log::info("Démarrage du client...\n");

    gf::TcpSocket socket("127.0.0.1", "5000");
    if (!socket)
    {
        gf::Log::error("Impossible de se connecter au serveur !\n");
        return -1;
    }

    char currentDir = 0;
    std::vector<ClientState> states;
    std::mutex statesMutex;
    bool running = true;
    socket.setNonBlocking();

    // ID local du client
    uint32_t myId = 0;

    // fonction pour générer une couleur basée sur l'ID
    auto colorFromId = [](uint32_t id) -> gf::Color4f
    {
        float r = float((id * 50) % 256) / 255.0f;
        float g = float((id * 80) % 256) / 255.0f;
        float b = float((id * 110) % 256) / 255.0f;
        return gf::Color4f(r, g, b, 1.0f);
    };

    std::thread receiver([&]()
                         {
        while (running && renderer.isOpen()) {
            gf::Packet packet;
        switch (socket.recvPacket(packet))
        {
        case gf::SocketStatus::Data:
            switch (packet.getType())
            {
            case GameState::type:
                auto data = packet.as<GameState>();
                states = data.clientStates;
                //ici, je suis censée récupérer le plateau(?)
                //mapS= data.plateau;
                break;
            }
            break;
        case gf::SocketStatus::Error:
             gf::Log::error("Erreur réseau côté client\n");
            break;
        case gf::SocketStatus::Close:
            gf::Log::info("Serveur déconnecté\n");
                running = false;
                renderer.getWindow().close();
            break;
        case gf::SocketStatus::Block:

            break;
        }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } });
    auto lastSend = std::chrono::steady_clock::now();

    int map_size = 27;
    std::vector<std::vector<int>> map(map_size, std::vector<int>(map_size, -1)); // provisoire : 0=floor 1=wall 2=hut

    map[0][0] = 1;
    map[1][0] = 1;
    map[2][0] = 1;
    map[3][0] = 1;
    map[4][0] = 1;
    map[5][0] = 1;
    map[6][0] = 1;
    map[7][0] = 1;
    map[8][0] = 1;
    map[9][0] = 1;
    map[10][0] = 1;
    map[11][0] = 1;
    map[12][0] = 1;
    map[13][0] = 1;

    map[0][1] = 1;
    map[0][2] = 1;
    map[0][3] = 1;
    map[0][4] = 1;
    map[0][5] = 1;
    map[0][6] = 1;
    map[0][7] = 1;

    map[2][3] = 2;

    map[4][4] = 0;

    mapRec mapS; //fausse map de serveur
    mapS.width = 27;
    mapS.height=27;
    int height =27;
    int width =27;
    mapS.grid.resize(mapS.height, std::vector<CaseRec>(mapS.width));


    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (y == 0 || y == height-1 || x == 0 || x == width-1) {
                CaseRec newC;
                newC.type=CellType::Wall;
                mapS.grid[y][x] = newC;
            } else if (y == height/2 && x == width/2) {
                CaseRec newC;
                newC.type=CellType::Hut;
                mapS.grid[y][x] = newC;
            } else {
                CaseRec newC;
                newC.type=CellType::Floor;
                mapS.grid[y][x] = newC;
            }
        }
    }

    while (running && renderer.isOpen())
    {
        gf::Event event;

        // Gestion des événements
        while (renderer.getWindow().pollEvent(event))
        {
            if (event.type == gf::EventType::Closed)
            {
                gf::Log::info("Fermeture demandée par l'utilisateur\n");
                running = false;
                renderer.getWindow().close();
            }
            else if (event.type == gf::EventType::KeyPressed)
            {
                switch (event.key.keycode)
                {
                case gf::Keycode::Up:
                    currentDir = 'U';
                    break;
                case gf::Keycode::Down:
                    currentDir = 'D';
                    break;
                case gf::Keycode::Left:
                    currentDir = 'L';
                    break;
                case gf::Keycode::Right:
                    currentDir = 'R';
                    break;
                default:
                    currentDir = 0;
                    break;
                }
            }
            else if (event.type == gf::EventType::KeyReleased)
            {
                currentDir = 0;
            }
        }

        // envoyer la touche avec un délai si maintenue
        if (currentDir != 0)
        {
            auto now = std::chrono::steady_clock::now();
            if (now - lastSend > std::chrono::milliseconds(100))
            {
                gf::Log::info("Envoi : touche '%c'\n", currentDir);
                gf::Packet packet;
                ClientMove cm;
                cm.moveDir = currentDir;
                packet.is(cm);
                socket.sendPacket(packet);
                // socket.sendRawBytes({reinterpret_cast<uint8_t*>(&currentDir), 1});
                lastSend = now;
            }
        }

        // Rendu
        renderer.render(states, myId, mapS);

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    if (receiver.joinable())
    {
        receiver.join();
    }

    gf::Log::info("Client fermé proprement\n");
    return 0;
}
