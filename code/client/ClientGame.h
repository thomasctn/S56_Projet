#pragma once

#include <gf/SceneManager.h>
#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>

#include "Renderer.h"
#include "../common/Types.h"
#include "WelcomeScene.h"
#include "LobbyScene.h"
#include "LobbyListScene.h"
#include "GameScene.h"
#include "EndScene.h"
#include "ClientNetworkHandler.h"
#include "ClientEventHandler.h"
#include "ClientScreen.h"


class ClientGame : public gf::SceneManager {

private: 

    Renderer m_renderer;


public:
    ClientGame();
    ~ClientGame();

    void run(const std::string& host = "127.0.0.1", const std::string& port = "5000"); //debut
    void shutdownClient(std::atomic<bool>& running);

    Renderer& getRenderer();
    
    gf::TcpSocket& getSocket();
    bool tryPopPacket(gf::Packet& out);
    void startNetwork(const std::string& host, const std::string& port);
    WelcomeScene welcomeScene; //toutes mes scenes
    LobbyListScene lobbyListScene;
    LobbyScene lobbyScene;
    GameScene gameScene;
    EndScene endScene;
    void setMyId(uint32_t id);
    uint32_t getMyId() const;



    
        

private:
    void connectToServer(const std::string& host, const std::string& port);
    void stopNetwork();

    gf::TcpSocket m_socket;
    std::atomic<bool> m_running;

    std::queue<gf::Packet> m_packetQueue;
    std::mutex m_packetMutex;
    std::thread m_receiver;

    std::vector<PlayerData> states;
    BoardCommon board;
    std::vector<std::pair<Position, PacGommeType>> pacgommes;
    unsigned int timeLeft = 999;
    int timeLeftPre = 0;
    std::map<Position, Position> holeLinks;

    RoomSettings roomSettings;
    PlayerRole myRole = PlayerRole::Spectator;
    int lastScore = 0;
    GameEndReason endReason = GameEndReason::TIME_OUT;
    ClientScreen screen = ClientScreen::Welcome;
    std::vector<PlayerData> lobbyPlayers;
    bool askedToJoin = false;
    bool amReady = false;
    uint32_t myId = 0;

    gf::ActionContainer actions;

    gf::Action upAction;
    gf::Action downAction;
    gf::Action leftAction;
    gf::Action rightAction;
};
