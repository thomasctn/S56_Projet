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
#include "../common/Constants.h"
#include "SceneRequest.h"
#include "WelcomeScene.h"
#include "LobbyScene.h"
#include "LobbyListScene.h"
#include "GameScene.h"
#include "EndScene.h"
#include "ClientNetworkHandler.h"


class ClientGame : public gf::SceneManager {



public:
    ClientGame();
    ~ClientGame();

    void run(const std::string& host = "127.0.0.1", const std::string& port = "5000"); //debut
    void shutdown();



    
    gf::TcpSocket& getSocket();

    void setLastRooms(const std::vector<RoomData>& rooms) {
        m_lastRooms = rooms;
    }

    const std::vector<RoomData>& getLastRooms() const {
        return m_lastRooms;
    }

    gf::ActionContainer& getActions(){ 
        return actions; 
    }
    void goToGameScene(const std::vector<PlayerData>& players, const BoardCommon& board,const std::map<Position, Position>& holeLinks);
    void goToEndScene(GameEndReason& endReason);
    bool tryPopPacket(gf::Packet& out);
    void startNetwork(const std::string& host, const std::string& port);
    WelcomeScene welcomeScene; //toutes mes scenes
    LobbyListScene lobbyListScene;
    LobbyScene lobbyScene;
    GameScene gameScene;
    EndScene endScene;
    void setMyId(uint32_t id);
    uint32_t getMyId() const;
    void requestScene(SceneRequest req);

    gf::Action upAction;
    gf::Action downAction;
    gf::Action leftAction;
    gf::Action rightAction;


    
        

private:
    void connectToServer(const std::string& host, const std::string& port);
    void stopNetwork();

    

    std::vector<RoomData> m_lastRooms;

    gf::TcpSocket m_socket;
    std::atomic<bool> m_running;
    SceneRequest m_sceneRequest = SceneRequest::None;

    std::queue<gf::Packet> m_packetQueue;
    std::mutex m_packetMutex;
    std::thread m_receiver;


    RoomSettings roomSettings;
    int lastScore = 0;
    uint32_t myId = 0;

    gf::ActionContainer actions;

};
