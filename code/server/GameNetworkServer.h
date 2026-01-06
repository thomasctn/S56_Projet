#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include <gf/TcpListener.h>
#include <gf/SocketSelector.h>
#include <gf/Packet.h>
#include <gf/Log.h>
#include <thread>
#include <chrono>
#include <iostream>
#include "../common/Serializable.h"
#include "Game.h"

class GameNetworkServer {
public:
    GameNetworkServer();

    int run();

    void stop() { running = false; }
    bool isRunning() const { return running; }

    Game& getGame();
    std::mutex& getPlayersMutex();

private:
    gf::TcpListener listener;
    gf::SocketSelector selector;

    Game game;
    std::mutex playersMutex;
    uint32_t nextId;

    std::atomic<bool> running;

    void handleNewClient();
    void handleClientData();
    void removeDisconnectedPlayers(const std::vector<uint32_t>& disconnectedIds);
    void broadcastStates();
};
