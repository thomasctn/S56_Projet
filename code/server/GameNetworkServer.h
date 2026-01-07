#pragma once

#include <gf/TcpListener.h>
#include <gf/SocketSelector.h>
#include <gf/Packet.h>
#include <gf/Log.h>

#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>
#include <queue>
#include <mutex>
#include <optional>

#include "../common/Serializable.h"
#include "../common/Constants.h"
#include "../common/Protocol.h"
#include "Game.h"
#include "InputQueue.h"



class GameNetworkServer {
public:
    GameNetworkServer();

    int run();

    void stop() { running = false; }
    bool isRunning() const { return running; }

    Game &getGame();
    std::mutex &getPlayersMutex();

    InputQueue inputQueue;


private:
    gf::TcpListener listener;
    gf::SocketSelector selector;

    Game game;
    std::mutex playersMutex;
    uint32_t nextId;

    std::atomic<bool> running;

    void handleNewClient();
    void handleClientData();
    void removeDisconnectedPlayers(const std::vector<uint32_t> &disconnectedIds);
    template <typename T>
    void broadcast(const T &data) {
        gf::Packet packet;
        packet.is(data);
        auto &players = game.getPlayers();

        for (auto &[id, playerPtr] : players)
        {
            playerPtr->socket.sendPacket(packet);
        }
    }
    void broadcastStates();
};


