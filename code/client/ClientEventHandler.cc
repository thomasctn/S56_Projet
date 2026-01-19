#include "ClientEventHandler.h"
#include <gf/Log.h>
#include <chrono>
#include <thread>

void shutdownClient(std::atomic<bool>& running);


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


void handleClientEvents(
    Renderer& renderer,
    gf::ActionContainer& actions,
    std::atomic<bool>& running,
    ClientScreen& screen,
    WelcomeScene& welcomeScene,
    LobbyScene& lobbyScene,
    EndScene& endScene,
    gf::TcpSocket& socket,
    bool& askedToJoin,
    bool& amReady,
    int& roomSize,
    int& nbBots,
    int& gameDur,
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


    }