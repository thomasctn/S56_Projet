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



void shutdownClient(std::atomic<bool>& running)
{
    if (!running.load()) return;

    gf::Log::info("Arrêt du client...\n");
    running.store(false);
}




void sendRoomSettings(gf::TcpSocket& socket,unsigned int newRoomSize, int newNbBots, int newGameDur){
    RoomSettings settings;
    settings.roomSize = newRoomSize;
    settings.nbBot = newNbBots;
    settings.gameDuration = newGameDur;

    gf::Packet p;
    p.is(ClientChangeRoomSettings{settings});
    socket.sendPacket(p);

    gf::Log::info("ClientChangeRoomSettings envoye : roomSize=%u\n", newRoomSize);
}

void updateMyRoleFromPlayers(const std::vector<PlayerData>& players,uint32_t myId,PlayerRole& myRole){
 

    for (const PlayerData& p : players) {
        if (p.id == myId) {
            myRole = p.role;
            gf::Log::info("Mon role maj : %d\n", int(myRole));
            return;
        }
    }
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


    //char currentDir = 0;
    //STRCUTURES DE DONNÉES RECUES
    std::vector<PlayerData> states;
    BoardCommon board;
    std::vector<std::pair<Position, PacGommeType>> pacgommes;
    unsigned int timeLeft = 999;
    int timeLeftPre = 0;

    int roomSize = int(MAX_PLAYERS); // capacité actuelle de la room (modifiable)
    int nbBots = int(NB_BOTS);//modifiable! //avec int parcque thomas a mis ses trucs en size_t et il sait pas?
    int gameDur = T_GAME;
    PlayerRole myRole;

    int lastScore;
    GameEndReason endReason;



    //std::mutex statesMutex;
    socket.setNonBlocking();

    //ajout pr un lobby
    enum class ClientScreen { 
        Welcome, 
        Lobby, 
        Playing,
        End
    };
    ClientScreen screen = ClientScreen::Welcome;

    int connectedPlayers = 0; //normalement mis a jour par ce que le serveur me dit selon benoit plus tard

    bool askedToJoin = false;  // pour n'envoyer la requête join qu'une fois
    bool amReady = false; //notre état local prêt/pas prêt


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

    //test fix ecrasement page
    auto size = renderer.getWindow().getSize();
    renderer.handleResize(size.x, size.y);

    //scene welcome
    WelcomeScene welcomeScene(renderer);
    LobbyScene lobbyScene(renderer);
    GameScene gameScene(renderer);

    EndScene endScene(renderer);
    //WelcomeEntity welcome(renderer);


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
    

    std::thread receiver([&]() {
        while (running.load()) {
            gf::Packet packet;

            switch (socket.recvPacket(packet)) {
                case gf::SocketStatus::Data: {
                    std::lock_guard<std::mutex> lock(packetMutex);
                    packetQueue.push(std::move(packet));
                    break;
                }

                case gf::SocketStatus::Close:
                    gf::Log::info("Serveur déconnecté\n");
                    running.store(false); 
                    break;

                case gf::SocketStatus::Error:
                    if (running.load()) {
                        gf::Log::error("Erreur réseau côté client\n");
                    }
                    break;

                default:
                    break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    auto lastSend = std::chrono::steady_clock::now();

    

    while (running)
    {
        gf::Event event;

        // Gestion des événements
        while (renderer.getWindow().pollEvent(event))
        {   
            actions.processEvent(event); //important actions
            //mettre un bouton si on est dans le welcome!
            if (screen == ClientScreen::Welcome) {
                if (welcomeScene.processEvent(event)) {
                    gf::Log::info("Bouton ENTRER cliqué (par WelcomeScene)\n");

                    if(!askedToJoin){
                        askedToJoin = true;
                        gf::Packet p;
                        p.is(ClientJoinRoom{});
                        socket.sendPacket(p);
                    }

                    screen = ClientScreen::Lobby;
                }
            }


            

            // interaction lobby via LobbyScene
            if (screen == ClientScreen::Lobby) {
                LobbyAction act = lobbyScene.processEvent(event);
                switch (act){
                    case LobbyAction::RoomDec:
                        if (roomSize > MIN_NB_PLAYERS){
                            roomSize--;
                            sendRoomSettings(socket, roomSize, nbBots, gameDur);
                        }
                        break;
                    case LobbyAction::RoomInc:
                        if (roomSize < MAX_NB_PLAYERS){
                            roomSize++;
                            sendRoomSettings(socket, roomSize, nbBots, gameDur);
                        }
                        break;
                    case LobbyAction::BotDec:
                        if (nbBots > MIN_NB_BOTS){
                            nbBots--;
                            sendRoomSettings(socket, roomSize, nbBots, gameDur);
                        }
                        break;
                    case LobbyAction::BotInc:
                        if (nbBots < MAX_NB_BOTS){
                            nbBots++;
                            sendRoomSettings(socket, roomSize, nbBots, gameDur);
                        }
                        break;
                    case LobbyAction::DurDec:
                        if (gameDur > MIN_DURATION){
                            gameDur = gameDur - 20;
                            sendRoomSettings(socket, roomSize, nbBots, gameDur);
                        }
                        break;
                    case LobbyAction::DurInc:
                        if (gameDur < MAX_DURATION){
                            gameDur = gameDur + 20;
                            sendRoomSettings(socket, roomSize, nbBots, gameDur);
                        }
                        break;
                    case LobbyAction::ToggleReady: {
                        amReady = !amReady;
                        gf::Packet p;
                        p.is(ClientReady{amReady});
                        socket.sendPacket(p);
                        break;
                    }
                    case LobbyAction::ChangeRole: {
                        PlayerData newData;
                        newData.id = myId;
                        newData.role = (myRole == PlayerRole::PacMan) ? PlayerRole::Ghost : PlayerRole::PacMan;
                        newData.ready = amReady;
                        newData.score = 0;
                        newData.x = 0.f;
                        newData.y = 0.f;
                        newData.color = 0;
                        newData.name = "";
                        gf::Packet p;
                        p.is(ClientChangeRoomCharacterData{ newData });
                        socket.sendPacket(p);
                        gf::Log::info("Demande changement de rôle envoyée (%d -> %d)\n", int(myRole), int(newData.role));
                        break;
                    }
                    default: break;
                }
            }   


            if (screen == ClientScreen::End) {
                if (endScene.processEvent(event)) {
                    gf::Log::info("Bouton retour au lobby cliqué (par EndScene)\n");

                    amReady = false;

                    screen = ClientScreen::Lobby;
                }
            }


            if (event.type == gf::EventType::Closed)
            {
                gf::Log::info("Fermeture demandée par l'utilisateur\n");
                shutdownClient(running);
                {
                    gf::TcpSocket tmpSocket = std::move(socket);
                }

                if (receiver.joinable())
                    receiver.join();

                renderer.getWindow().close();
            }

            else if (event.type == gf::EventType::Resized) { //changement taille
                auto size = renderer.getWindow().getSize();
                renderer.handleResize(size.x, size.y);
            }
            
        }

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
            std::lock_guard<std::mutex> lock(packetMutex);
            while (!packetQueue.empty()) {
                gf::Packet packet = std::move(packetQueue.front());
                packetQueue.pop();
                switch (packet.getType()) {
                    //ce serait bien que je recoive qqch quand quelque était en jeu et se deconnecte
                    //comme ça je peux nous retourner les autre a l'écran d'acceuil! 
                    //en fait pas sur ptet thomas il ajoute des bots?

                    case ServerRoomSettings::type:{
                        auto data = packet.as<ServerRoomSettings>();
                        roomSize = int(data.settings.roomSize);
                        nbBots = int(data.settings.nbBot);
                        gameDur = int(data.settings.gameDuration);
                        gf::Log::info("ServerRoomSettings reçu : roomSize=%u, nbBots=%u\n", data.settings.roomSize,data.settings.nbBot);
                        break;
                    }

                    case ServerJoinRoom::type: {
                        gf::Log::info("Serveur: rejoint la room\n"); //pour l'instant aussi inutile (le serv nous met direct dans une room)
                        break;
                    }

                    case ServerConnect::type: {
                        auto data = packet.as<ServerConnect>();
                        myId = data.clientId;

                        gf::Log::info("ID client assigné par le serveur : %u\n", myId);
                        break;
                    }


                    case ServerListRoomPlayers::type: {
                        auto data = packet.as<ServerListRoomPlayers>();
                        connectedPlayers = data.players.size();
                        updateMyRoleFromPlayers(data.players, myId, myRole);
                        gf::Log::info("Lobby: %d / %d joueurs\n", connectedPlayers, roomSize);
                        break;
                    }

                    case ServerGamePreStart::type:{
                        auto data = packet.as<ServerGamePreStart>();
                        timeLeftPre = int(data.timeLeft);
                        break;
                    }

                    case ServerGameStart::type:{
                        auto data = packet.as<ServerGameStart>();

                        states = data.players;  
                        board=data.board;

                        updateMyRoleFromPlayers(data.players, myId, myRole);

                        screen = ClientScreen::Playing; //passage en mode jeu, on montre la carte
                        
                        gf::Log::info("Game start reçu -> passage en Playing\n");
                        break;
                    }

                    case ServerGameState::type:{
                        auto data = packet.as<ServerGameState>();
                        states = data.clientStates;
                        //board=data.board; (on me l'envoie mais je suis pas censé le prendre a chaque fois)
                        pacgommes.clear();
                        for (auto& [pos, type] : data.pacgommes) {
                            pacgommes.push_back({pos, type});
                        }
                        timeLeft = data.timeLeft;

                        for (const auto& p : data.clientStates) {
                            if (p.role == PlayerRole::PacMan) {
                                lastScore = p.score;
                                break;
                            }
                        }
                        break;
                    }

                    case ServerGameEnd::type:{
                        auto data = packet.as<ServerGameEnd>();
                        screen = ClientScreen::End;
                        amReady=false;
                        endReason = data.reason;
                        //ce serait bien que je puisse récup le score et qui a gagné!
                        break;
                    }

                    
                }
            }
        }


        


        // Rendu basique
        //renderer.render(states, myId, board);

        if(screen == ClientScreen::Welcome) {
            welcomeScene.render();
        }
        else if(screen == ClientScreen::Lobby) {
            //renderer.drawLobby(connectedPlayers, maxPlayers); 
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
