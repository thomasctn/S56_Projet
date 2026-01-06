#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Shapes.h>

#include <gf/Action.h>
#include <gf/Color.h>
#include <gf/Log.h>
#include <thread>
#include <chrono>
#include <vector>
#include <gf/Keyboard.h>
#include <gf/Event.h>
#include <mutex>
#include <queue>
#include "../common/Types.h"
#include "Renderer.h"


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

    //char currentDir = 0;
    //STRCUTURES DE DONNÉES RECUES
    std::vector<PlayerData> states;
    BoardCommon board;

    //std::mutex statesMutex;
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

    gf::ActionContainer actions;

    //directions
    gf::Action upAction("Up");
    upAction.addKeycodeKeyControl(gf::Keycode::Up);
    upAction.setContinuous();
    actions.addAction(upAction);

    gf::Action downAction("Down");
    downAction.addKeycodeKeyControl(gf::Keycode::Down);
    downAction.setContinuous();
    actions.addAction(downAction);

    gf::Action leftAction("Left");
    leftAction.addKeycodeKeyControl(gf::Keycode::Left);
    leftAction.setContinuous();
    actions.addAction(leftAction);

    gf::Action rightAction("Right");
    rightAction.addKeycodeKeyControl(gf::Keycode::Right);
    rightAction.setContinuous();
    actions.addAction(rightAction);

    std::queue<gf::Packet> packetQueue;
    std::mutex packetMutex;
    

    std::thread receiver([&]()
                         {
        while (running && renderer.isOpen()) {
            gf::Packet packet;
        switch (socket.recvPacket(packet))
        {
        case gf::SocketStatus::Data:{
            /*switch (packet.getType())
            {
            case GameState::type:
                auto data = packet.as<GameState>();
                states = data.PlayerDatas;
                gf::Log::info("Donnée reçu\n");

                //ici, je suis censée récupérer le plateau(?)
                //mapS= data.plateau;
                break;
            }*/
            std::lock_guard<std::mutex> lock(packetMutex);
            packetQueue.push(std::move(packet));
            break;
        }
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

    


    while (running && renderer.isOpen())
    {
        gf::Event event;

        // Gestion des événements
        while (renderer.getWindow().pollEvent(event))
        {   
            actions.processEvent(event); //important actions
            if (event.type == gf::EventType::Closed)
            {
                gf::Log::info("Fermeture demandée par l'utilisateur\n");
                running = false;
                renderer.getWindow().close();
            }
            else if (event.type == gf::EventType::Resized) { //changement taille
                auto size = renderer.getWindow().getSize();
                renderer.handleResize(size.x, size.y);
            }
            
        }

        
        //envoi réseau de la touche
        auto now = std::chrono::steady_clock::now();
        if (now-lastSend > std::chrono::milliseconds(100)) {

            char dir=0;

            if (upAction.isActive()) {
                dir = 'U';
            } else if (downAction.isActive()) {
                dir = 'D';
            } else if (leftAction.isActive()) {
                dir = 'L';
            } else if (rightAction.isActive()) {
                dir = 'R';
            }

            if(dir != 0){
                gf::Log::info("Envoi : touche '%c'\n", dir);
            
                gf::Packet packet;
                ClientMove cm;
                cm.moveDir = dir;
                packet.is(cm);
                socket.sendPacket(packet);
                lastSend=now;
            }
        }
        //dépiler les packets du réseau
        {
            std::lock_guard<std::mutex> lock(packetMutex);
            while (!packetQueue.empty()) {
                gf::Packet packet = std::move(packetQueue.front());
                packetQueue.pop();
                switch (packet.getType()) {
                    case GameState::type: {
                        auto data = packet.as<GameState>();
                        states = data.clientStates;
                        board=data.board;
                        break;
                    }
                }
            }
        }

        // Rendu
        renderer.render(states, myId, board);
        

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    if (receiver.joinable())
    {
        receiver.join();
    }

    gf::Log::info("Client fermé proprement\n");
    return 0;
}
