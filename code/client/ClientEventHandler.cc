#include "ClientEventHandler.h"
#include <gf/Log.h>
#include <chrono>
#include <thread>

void shutdownClient(std::atomic<bool>& running);


void sendRoomSettings(gf::TcpSocket& socket,unsigned int newRoomSize, int newNbBots, int newGameDur){
    RoomSettings newSettings;
    newSettings.roomSize = newRoomSize;
    newSettings.nbBot = newNbBots;
    newSettings.gameDuration = newGameDur;

    gf::Packet p;
    p.is(ClientChangeRoomSettings{newSettings});
    socket.sendPacket(p);

    gf::Log::info("ClientChangeRoomSettings envoye : roomSettings.roomSize=%u\n", newSettings.roomSize);
}


void handleClientEvents(
    Renderer& renderer,
    gf::ActionContainer& actions,
    std::atomic<bool>& running,
    ClientScreen& screen,
    WelcomeScene& welcomeScene,
    LobbyScene& lobbyScene,
    EndScene& endScene,
    LobbyListScene& lobbyListScene,
    gf::TcpSocket& socket,
    bool& askedToJoin,
    bool& amReady,
    RoomSettings& roomSettings,
    PlayerRole& myRole,
    uint32_t myId,
    std::thread& receiver
) {
    gf::Event event;
        
    while (renderer.getWindow().pollEvent(event))
            {   
                actions.processEvent(event); //important actions
                //mettre un bouton si on est dans le welcome!
                if (screen == ClientScreen::Welcome) {
                    if (welcomeScene.processEvent(event)) {
                        gf::Log::info("Bouton ENTRER cliqué (par WelcomeScene)\n");

                     

                        screen = ClientScreen::LobbyList;
                    }
                }

                else if (screen == ClientScreen::LobbyList) {
                    LobbyListAction act = lobbyListScene.processEvent(event);

                    if (act == LobbyListAction::CreateRoom) {
                        gf::Packet p;
                        ClientCreateRoom create; //ça suffit? apparement oui
                        p.is(create);
                        socket.sendPacket(p);
                    }
                    else if (act == LobbyListAction::JoinRoom) {
                        unsigned int roomId = lobbyListScene.getLastRoomId();

                        gf::Packet p;
                        ClientJoinRoom join;
                        join.room = gf::Id(roomId);
                        p.is(join);
                        socket.sendPacket(p);
                    }
                }   



                

                // interaction lobby via LobbyScene
                else if (screen == ClientScreen::Lobby) {
                    LobbyAction act = lobbyScene.processEvent(event);
                    switch (act){
                        case LobbyAction::Leave: {
                            gf::Packet p;
                            p.is(ClientLeaveRoom{});
                            socket.sendPacket(p);
                            break;
                        }
                        case LobbyAction::RoomDec:
                            if (roomSettings.roomSize > MIN_NB_PLAYERS){
                                int newRoomSize = roomSettings.roomSize - 1;
                                sendRoomSettings(socket, newRoomSize, roomSettings.nbBot, roomSettings.gameDuration);
                            }
                            break;
                        case LobbyAction::RoomInc:
                            if (roomSettings.roomSize < MAX_NB_PLAYERS){
                                int newRoomSize = roomSettings.roomSize +1;
                                sendRoomSettings(socket, newRoomSize, roomSettings.nbBot, roomSettings.gameDuration);
                            }
                            break;
                        case LobbyAction::BotDec:
                            if (roomSettings.nbBot > MIN_NB_BOTS){
                                int newNbBots = roomSettings.nbBot -1;
                                sendRoomSettings(socket, roomSettings.roomSize, newNbBots, roomSettings.gameDuration);
                            }
                            break;
                        case LobbyAction::BotInc:
                            if (roomSettings.nbBot < MAX_NB_BOTS){
                                int newNbBots = roomSettings.nbBot +1;
                                sendRoomSettings(socket, roomSettings.roomSize, newNbBots, roomSettings.gameDuration);
                            }
                            break;
                        case LobbyAction::DurDec:
                            if (roomSettings.gameDuration > MIN_DURATION){
                                int newGameDur = roomSettings.nbBot -20;
                                sendRoomSettings(socket, roomSettings.roomSize, roomSettings.nbBot, newGameDur);
                            }
                            break;
                        case LobbyAction::DurInc:
                            if (roomSettings.gameDuration < MAX_DURATION){
                                int newGameDur = roomSettings.nbBot +20;
                                sendRoomSettings(socket, roomSettings.roomSize, roomSettings.nbBot, newGameDur);
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


                else if (screen == ClientScreen::End) {
                    if (endScene.processEvent(event)) {
                        gf::Log::info("Bouton retour au lobby cliqué (par EndScene)\n");

                        amReady = false;

                        screen = ClientScreen::Lobby;
                    }
                }


                else if (event.type == gf::EventType::Closed)
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


    }