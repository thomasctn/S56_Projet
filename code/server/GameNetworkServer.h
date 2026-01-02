#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include <gf/TcpListener.h>
#include <gf/SocketSelector.h>
#include <gf/Packet.h>
#include "Game.h"
#include "ClientInfo.h"

class GameNetworkServer {
public:
    GameNetworkServer();

    int run();

    void stop() { running = false; }
    bool isRunning() const { return running; }

    Game& getGame();
    std::vector<ClientInfo>& getClients();
    std::mutex& getClientsMutex();

private:
    gf::TcpListener listener;
    gf::SocketSelector selector;

    std::vector<ClientInfo> clients;
    std::mutex clientsMutex;
    uint32_t nextId;

    Game game;

    std::atomic<bool> running;

    void handleNewClient();
    void handleClientData();
    void removeDisconnectedClients(const std::vector<size_t>& toRemove);
    void broadcastStates();
};
