#pragma once

#include <unordered_map>
#include <cstdint>
#include <memory>
#include <gf/TcpListener.h>
#include <gf/SocketSelector.h>
#include <gf/Packet.h>
#include "ServerNetwork.h"
#include "Lobby.h"
#include "../common/Protocol.h"
#include <gf/Log.h>


struct PlayerConnection {
    uint32_t id;
    gf::TcpSocket socket;
};


class Lobby;

class ServerNetwork {
public:
    ServerNetwork();
    int run();
    void send(uint32_t playerId, gf::Packet& packet);
    Lobby& getLobby() { return *lobby; }
    void stop();

private:
    void handleNewClient();
    void handleClientData();
    void disconnectPlayer(uint32_t id);

private:
    gf::TcpListener listener;
    gf::SocketSelector selector;
    std::unordered_map<uint32_t, PlayerConnection> connections;
    uint32_t nextId;
    bool running;

    std::unique_ptr<Lobby> lobby;
};
