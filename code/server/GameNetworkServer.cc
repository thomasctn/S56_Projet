#include "GameNetworkServer.h"
#include "../common/Serializable.h"
#include <gf/Log.h>
#include <thread>
#include <chrono>
#include <iostream>

GameNetworkServer::GameNetworkServer()
    : listener("5000"), nextId(1), game(27, 27), running(true)
{
    if (!listener)
    {
        gf::Log::error("Impossible de démarrer le serveur !");
    }
    selector.addSocket(listener);
    /*
    std::cout << "Plateau initial :" << std::endl;
    game.getPlateau().print();
    */
}

int GameNetworkServer::run()
{
    gf::Log::info("Serveur démarré !");
    while (running) {
        if (selector.wait(gf::milliseconds(100)) == gf::v1::SocketSelectorStatus::Event) {
            handleNewClient();
            handleClientData();
        }
    }
    gf::Log::info("Serveur arrêté proprement\n");
    return 0;
}

void GameNetworkServer::handleNewClient()
{
    if (!selector.isReady(listener))
        return;

    gf::TcpSocket clientSocket = listener.accept();
    if (!clientSocket)
        return;

    std::lock_guard<std::mutex> lock(clientsMutex);
    ClientInfo ci;
    ci.id = nextId++;
    ci.socket = std::move(clientSocket);
    ci.state = {ci.id, 100.0f, 100.0f, 0xFF0000FF};
    clients.push_back(std::move(ci));
    selector.addSocket(clients.back().socket);

    game.addPlayer(clients.back().id, clients.back().state.x, clients.back().state.y);

    gf::Log::info("Client connecté, id=%d\n", clients.back().id);
}

void GameNetworkServer::handleClientData()
{
    std::vector<size_t> toRemove;

    std::lock_guard<std::mutex> lock(clientsMutex);
    for (size_t i = 0; i < clients.size(); ++i)
    {
        auto &c = clients[i];
        if (!selector.isReady(c.socket))
            continue;

        gf::Packet packet;
        switch (c.socket.recvPacket(packet))
        {
        case gf::SocketStatus::Data:
            switch (packet.getType())
            {
            case ClientMove::type:
            {
                auto data = packet.as<ClientMove>();
                char dir = data.moveDir;

                Direction direction;
                switch (dir)
                {
                case 'U': direction = Direction::Up;    break;
                case 'D': direction = Direction::Down;  break;
                case 'L': direction = Direction::Left;  break;
                case 'R': direction = Direction::Right; break;
                default:
                    break;
                }

                game.requestMove(c.id, direction);

                auto &gPlayer = game.getPlayerInfo(c.id);
                c.state.x = gPlayer.x;
                c.state.y = gPlayer.y;

                gf::Log::info(
                    "Client %d moved %c -> position=(%.1f, %.1f)\n",
                    c.id, dir, c.state.x, c.state.y
                );
                break;
            }
            }
            break;

        case gf::SocketStatus::Error:
        case gf::SocketStatus::Close:
            toRemove.push_back(i);
            break;

        case gf::SocketStatus::Block:
            break;
        }
    }

    removeDisconnectedClients(toRemove);
    broadcastStates();
}

void GameNetworkServer::removeDisconnectedClients(const std::vector<size_t> &toRemove)
{
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it)
    {
        selector.removeSocket(clients[*it].socket);
        gf::Log::info("Client %d déconnecté\n", clients[*it].id);
        clients.erase(clients.begin() + *it);
    }
}

void GameNetworkServer::broadcastStates()
{
    for (auto &c : clients)
    {
        std::vector<ClientState> clientStates;
        for (auto &other : clients)
        {
            clientStates.push_back(other.state);
        }
        gf::Packet packet;
        GameState gs;
        gs.clientStates = clientStates;
        packet.is(gs);
        c.socket.sendPacket(packet);
        gf::Log::info("Envoi des état au client %d\n", c.id);

        /* TODO
        PLATEAU A ENVOYER
        const Plateau& p = game.getPlateau();
        */


    }
}

Game& GameNetworkServer::getGame() {
    return game;
}

std::vector<ClientInfo>& GameNetworkServer::getClients() {
    return clients;
}

std::mutex& GameNetworkServer::getClientsMutex() {
    return clientsMutex;
}
