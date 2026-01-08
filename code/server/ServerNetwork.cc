#include "ServerNetwork.h"

ServerNetwork::ServerNetwork()
: listener("5000")
, nextId(1)
, running(true)
, lobby(std::make_unique<Lobby>(*this)) // construction correcte
{
    if (!listener) {
        gf::Log::error("Impossible de démarrer le serveur réseau\n");
    }

    selector.addSocket(listener);
}

int ServerNetwork::run() {
    gf::Log::info("ServerNetwork démarré\n");

    while (running) {
        if (selector.wait(gf::milliseconds(10)) == gf::SocketSelectorStatus::Event) {
            handleNewClient();
            handleClientData();
        }
    }

    gf::Log::info("ServerNetwork arrêté proprement\n");
    return 0;
}

void ServerNetwork::handleNewClient() {
    if (!selector.isReady(listener))
        return;

    gf::TcpSocket socket = listener.accept();
    if (!socket)
        return;

    uint32_t id = nextId++;

    PlayerConnection connection;
    connection.id = id;
    connection.socket = std::move(socket);

    selector.addSocket(connection.socket);
    connections.emplace(id, std::move(connection));

    lobby->onPlayerConnected(id);

    gf::Log::info("Client connecté (id=%u)\n", id);
}

void ServerNetwork::handleClientData() {
    std::vector<uint32_t> disconnected;

    for (auto& [id, conn] : connections) {
        if (!selector.isReady(conn.socket))
            continue;

        gf::Packet packet;
        switch (conn.socket.recvPacket(packet)) {

            case gf::SocketStatus::Data: {
                PacketContext ctx;
                ctx.senderId = id;
                ctx.packet = std::move(packet);

                lobby->handlePacket(ctx);
                break;
            }

            case gf::SocketStatus::Close:
            case gf::SocketStatus::Error:
                disconnected.push_back(id);
                break;

            case gf::SocketStatus::Block:
                break;
        }
    }

    for (uint32_t id : disconnected) {
        disconnectPlayer(id);
    }
}

void ServerNetwork::disconnectPlayer(uint32_t id) {
    auto it = connections.find(id);
    if (it == connections.end())
        return;

    selector.removeSocket(it->second.socket);

    lobby->onPlayerDisconnected(id);

    connections.erase(it);

    gf::Log::info("Client %u déconnecté\n", id);
}

void ServerNetwork::send(uint32_t playerId, gf::Packet& packet) {
    auto it = connections.find(playerId);
    if (it == connections.end())
        return;

    it->second.socket.sendPacket(packet);
}

void ServerNetwork::stop() {
    running = false;
    gf::Log::info("ServerNetwork arrêté\n");
}