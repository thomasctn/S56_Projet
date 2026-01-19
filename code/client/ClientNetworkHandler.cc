#include "ClientNetworkHandler.h"
#include <gf/Log.h>

void updateMyRoleFromPlayers(const std::vector<PlayerData>& players,uint32_t myId,PlayerRole& myRole){
 

    for (const PlayerData& p : players) {
        if (p.id == myId) {
            myRole = p.role;
            gf::Log::info("Mon role maj : %d\n", int(myRole));
            return;
        }
    }
}
void handleNetworkPackets(
    std::queue<gf::Packet>& packetQueue,
    std::mutex& packetMutex,
    ClientScreen& screen,
    std::vector<PlayerData>& states,
    BoardCommon& board,
    std::vector<std::pair<Position, PacGommeType>>& pacgommes,
    unsigned int& timeLeft,
    int& timeLeftPre,
    std::vector<PlayerData>& lobbyPlayers,
    RoomSettings& roomSettings,
    LobbyListScene& lobbyListScene,
    uint32_t& myId,
    PlayerRole& myRole,
    int& lastScore,
    GameEndReason& endReason,
    bool amReady
){

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
                        roomSettings.roomSize = int(data.settings.roomSize);
                        roomSettings.nbBot = int(data.settings.nbBot);
                        roomSettings.gameDuration = int(data.settings.gameDuration);
                        gf::Log::info("ServerRoomSettings reçu : roomSize=%u, nbBots=%u\n", data.settings.roomSize,data.settings.nbBot);
                        break;
                    }

                    case ServerJoinRoom::type: {
                        gf::Log::info("Serveur: rejoint la room\n"); //pour l'instant aussi inutile (le serv nous met direct dans une room)
                        //normalement c'est ici qu'on nous dit ok pr rejoindre la room et que 
                        //l'affichage passe a lobby

                        screen = ClientScreen::Lobby;
                        
                        break;
                    }

                    case ServerListRooms::type: {
                        auto data = packet.as<ServerListRooms>();
                        lobbyListScene.setRooms(data.rooms);
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
                        lobbyPlayers = data.players;
                        updateMyRoleFromPlayers(lobbyPlayers, myId, myRole);
                        gf::Log::info("Lobby: %d / %d joueurs\n", lobbyPlayers, roomSettings.roomSize);
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

std::thread startNetworkReceiver(
    gf::TcpSocket& socket,
    std::atomic<bool>& running,
    std::queue<gf::Packet>& packetQueue,
    std::mutex& packetMutex
) {
    return std::thread([&]() {
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
}