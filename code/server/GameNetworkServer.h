#pragma once
#include "ClientInfo.h"
#include <vector>
#include <mutex>
#include <gf/TcpListener.h>
#include <gf/SocketSelector.h>
#include "Game.h"


class GameNetworkServer {
public:
    GameNetworkServer();
    int run();

private:
    gf::TcpListener listener;
    gf::SocketSelector selector;

    std::vector<ClientInfo> clients;
    std::mutex clientsMutex;
    uint32_t nextId;

    Game game;

    void handleNewClient();
    void handleClientData();
    void removeDisconnectedClients(const std::vector<size_t>& toRemove);
    void broadcastStates();
};
