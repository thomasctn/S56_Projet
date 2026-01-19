#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Shapes.h>

#include <gf/Action.h>
#include <gf/Color.h>
#include <gf/Log.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <vector>
#include <gf/Keyboard.h>
#include <gf/Event.h>
#include <mutex>
#include <queue>
#include "../common/Types.h"
#include "../common/Constants.h"
#include "Renderer.h"
#include "WelcomeScene.h"
#include "WelcomeEntity.h"
#include "LobbyScene.h"
#include "GameScene.h"
#include "GameEntity.h"
#include "EndScene.h"
#include "ClientNetworkHandler.h"
#include "ClientEventHandler.h"
#include "ClientScreen.h"



void shutdownClient(std::atomic<bool>& running)
{
    if (!running.load()) return;

    gf::Log::info("Arrêt du client...\n");
    running.store(false);
}


Renderer renderer;

int main()
{
    gf::Log::info("Démarrage du client...\n");

    gf::TcpSocket socket;
    bool connected = false;
    std::atomic<bool> running{true};


    while (!connected && running.load()) {
        socket = gf::TcpSocket("127.0.0.1", "5000");
        if (socket) {
            connected = true;
            gf::Log::info("Connecté au serveur !\n");
        } else {
            gf::Log::info("Impossible de se connecter au serveur, nouvelle tentative dans 1s...\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }


    //STRCUTURES DE DONNÉES RECUES/PERSO
    std::vector<PlayerData> states;
    BoardCommon board;
    std::set<Position> pacgommes;
    unsigned int timeLeft = 999;
    int timeLeftPre = 0;

    int roomSize = int(MAX_PLAYERS); // capacité actuelle de la room (modifiable)
    int nbBots = int(NB_BOTS);//modifiable! //avec int parcque thomas a mis ses trucs en size_t et il sait pas?
    int gameDur = T_GAME;
    PlayerRole myRole;

    int lastScore;
    GameEndReason endReason;

    ClientScreen screen = ClientScreen::Welcome;

    int connectedPlayers = 0; //normalement mis a jour par ce que le serveur me dit selon benoit plus tard

    bool askedToJoin = false;  // pour n'envoyer la requête join qu'une fois
    bool amReady = false; //notre état local prêt/pas prêt

    uint32_t myId = 0; // ID local du client



    //std::mutex statesMutex;
    socket.setNonBlocking();


    //fix ecrasement page
    auto size = renderer.getWindow().getSize();
    renderer.handleResize(size.x, size.y);

    //scenes
    WelcomeScene welcomeScene(renderer);
    LobbyScene lobbyScene(renderer);
    GameScene gameScene(renderer);
    EndScene endScene(renderer);


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

    std::thread receiver = startNetworkReceiver(
        socket,
        running,
        packetQueue,
        packetMutex
    );  

    auto lastSend = std::chrono::steady_clock::now();

    

    while (running)
    {
        gf::Event event;

        // Gestion des événements
        handleClientEvents(
            renderer,
            actions,
            running,
            screen,
            welcomeScene,
            lobbyScene,
            endScene,
            socket,
            askedToJoin,
            amReady,
            roomSize,
            nbBots,
            gameDur,
            myRole,
            myId,
            receiver
        );

        

        if (!running.load())
            break;

        
        //envoi réseau de la touche
        if (screen == ClientScreen::Playing){ //envoie seulement si on est en jeu!!
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
        }
        //dépiler les packets du réseau
        {
            handleNetworkPackets(
                packetQueue,
                packetMutex,
                screen,
                states,
                board,
                pacgommes,
                timeLeft,
                timeLeftPre,
                connectedPlayers,
                roomSize,
                nbBots,
                gameDur,
                myId,
                myRole,
                lastScore,
                endReason,
                amReady
            );

        }


        


        // Rendu basique

        if(screen == ClientScreen::Welcome) {
            welcomeScene.render();
        }
        else if(screen == ClientScreen::Lobby) {
            lobbyScene.render(connectedPlayers, roomSize, amReady, nbBots, gameDur, myRole);
        }
        else if(screen == ClientScreen::End) {
            endScene.render(lastScore, endReason);
        }
        else{ //Playing
            gameScene.render(states, myId, board, pacgommes, timeLeftPre, timeLeft);
        }
        

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    if (renderer.getWindow().isOpen()){
        renderer.getWindow().close();
    }

    if (receiver.joinable())
    {
        receiver.join();
    }

    gf::Log::info("Client fermé proprement\n");
    return 0;
}
