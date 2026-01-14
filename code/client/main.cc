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
#include "../common/Constants.h"
#include "Renderer.h"
#include "WelcomeScene.h"
#include "WelcomeEntity.h"



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
    std::set<Position> pacgommes;

    int roomSize = int(MAX_PLAYERS); // capacité actuelle de la room (modifiable)
    int nbBots = int(NB_BOTS);//modifiable! //avec int parcque thomas a mis ses trucs en size_t et il sait pas?
    int gameDur = T_GAME;
    PlayerRole myRole;


    //std::mutex statesMutex;
    bool running = true;
    socket.setNonBlocking();

    //ajout pr un lobby
    enum class ClientScreen { 
        Welcome, 
        Lobby, 
        Playing
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
    

    std::thread receiver([&]()
                         {
        while (running && renderer.isOpen()) {
            gf::Packet packet;
        switch (socket.recvPacket(packet))
        {
        case gf::SocketStatus::Data:{
            
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


            

            //boutons - et + du lobby
            if(event.type == gf::EventType::MouseButtonPressed && event.mouseButton.button == gf::MouseButton::Left){
                auto winSize = renderer.getWindow().getSize(); 
                float mx = event.mouseButton.coords.x * (renderer.getWorldSize() / float(winSize.x));
                float my = event.mouseButton.coords.y * (renderer.getWorldSize() / float(winSize.y));

                //bouton -
                auto minusPos = renderer.getMinusBtnPos();
                auto btnSize = renderer.getBtnSize();
                if(mx >= minusPos.x && mx <= minusPos.x + btnSize.x &&my >= minusPos.y && my <= minusPos.y + btnSize.y){
                    if(roomSize > MIN_NB_PLAYERS) roomSize--;
                    sendRoomSettings(socket, roomSize, nbBots, gameDur);
                }

                //bouton +
                auto plusPos = renderer.getPlusBtnPos();
                if(mx >= plusPos.x && mx <= plusPos.x + btnSize.x &&my >= plusPos.y && my <= plusPos.y + btnSize.y){
                    if(roomSize < MAX_NB_PLAYERS) roomSize++;
                    sendRoomSettings(socket, roomSize,nbBots, gameDur);
                }

                //bouton - bot
                auto minusBotPos = renderer.getMinusBotBtnPos();
                if(mx >= minusBotPos.x && mx <= minusBotPos.x + btnSize.x &&my >= minusBotPos.y && my <= minusBotPos.y + btnSize.y){
                    if(nbBots > MIN_NB_BOTS) nbBots--;
                    sendRoomSettings(socket, roomSize, nbBots, gameDur);
                }

                //bouton + bot
                auto plusBotPos = renderer.getPlusBotBtnPos();
                if(mx >= plusBotPos.x && mx <= plusBotPos.x + btnSize.x &&my >= plusBotPos.y && my <= plusBotPos.y + btnSize.y){
                    if(nbBots < MAX_NB_BOTS) nbBots++;
                    sendRoomSettings(socket, roomSize,nbBots, gameDur);
                }

                //bouton - durée
                auto minusDurPos = renderer.getMinusDurBtnPos();
                if(mx >= minusDurPos.x && mx <= minusDurPos.x + btnSize.x &&my >= minusDurPos.y && my <= minusDurPos.y + btnSize.y){
                    if(gameDur > MIN_DURATION) gameDur=gameDur-20;
                    sendRoomSettings(socket, roomSize, nbBots, gameDur);
                }

                //bouton + durée
                auto plusDurPos = renderer.getPlusDurBtnPos();
                if(mx >= plusDurPos.x && mx <= plusDurPos.x + plusDurPos.x &&my >= plusDurPos.y && my <= plusDurPos.y + btnSize.y){
                    if(gameDur < MAX_DURATION) gameDur=gameDur+20;;
                    sendRoomSettings(socket, roomSize,nbBots, gameDur);
                }

                //bouton PRET
                auto readyPos = renderer.getReadyBtnPos();
                auto readySize = renderer.getReadyBtnSize();
                if(mx >= readyPos.x && mx <= readyPos.x + readySize.x &&my >= readyPos.y && my <= readyPos.y + readySize.y){
                    amReady = !amReady;
                    gf::Packet p;
                    p.is(ClientReady{amReady});
                    socket.sendPacket(p);
                }

                //bouton changement de role
                auto CRPos = renderer.getChangeRoleBtnPos();
                if(mx >= CRPos.x && mx <= CRPos.x + readySize.x &&my >= CRPos.y && my <= CRPos.y + readySize.y){
                    PlayerData newData;
                    newData.id = myId;

                    // on inverse le roel
                    if(myRole == PlayerRole::PacMan){
                        newData.role = PlayerRole::Ghost;
                    }else{
                        newData.role = PlayerRole::PacMan;
                    }
                    // on fout nimporte quoi dans le reste
                    newData.ready = amReady;
                    newData.score = 0;//le reste est juste la car je dois le remplir faut que server ignore
                    newData.x = 0.f;   
                    newData.y = 0.f;
                    newData.color = 0;  
                    newData.name = "";

                    gf::Packet p;
                    p.is(ClientChangeRoomCharacterData{ newData });
                    socket.sendPacket(p);

                    gf::Log::info("Demande changement de rôle envoyée (%d -> %d)\n",int(myRole),int(newData.role));
                }

            }




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
                        pacgommes = data.pacgommes;
                        break;
                    }

                    case ServerGameEnd::type:{
                        auto data = packet.as<ServerGameEnd>();
                        screen = ClientScreen::Lobby;
                        amReady=false;
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
            renderer.renderLobby(connectedPlayers, roomSize, amReady, nbBots, gameDur, myRole);
        }
        else{ //Playing
            renderer.render(states, myId, board, pacgommes);
        }
        

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    if (receiver.joinable())
    {
        receiver.join();
    }

    gf::Log::info("Client fermé proprement\n");
    return 0;
}
