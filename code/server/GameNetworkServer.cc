#include "GameNetworkServer.h"
#include "../common/Serializable.h"
#include <gf/Log.h>
#include <thread>
#include <chrono>
#include <iostream>


GameNetworkServer::GameNetworkServer()
: listener("5000")
, nextId(1)
, game(27, 27)
{
    if (!listener) {
        gf::Log::error("Impossible de démarrer le serveur !");
    }
    selector.addSocket(listener);
    /*
    std::cout << "Plateau initial :" << std::endl;
    game.getPlateau().print();
    */
    std::thread(&GameNetworkServer::plateauDisplayLoop, this).detach();

}

int GameNetworkServer::run() {
    gf::Log::info("Serveur démarré !");
    while (true) {
        if (selector.wait(gf::milliseconds(100)) == gf::v1::SocketSelectorStatus::Event) {
            handleNewClient();
            handleClientData();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return 0;
}

void GameNetworkServer::handleNewClient() {
    if (!selector.isReady(listener)) return;

    gf::TcpSocket clientSocket = listener.accept();
    if (!clientSocket) return;

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


void GameNetworkServer::handleClientData() {
    std::vector<size_t> toRemove;

    std::lock_guard<std::mutex> lock(clientsMutex);
    for (size_t i = 0; i < clients.size(); ++i) {
        auto& c = clients[i];
        if (!selector.isReady(c.socket)) continue;

        std::vector<uint8_t> buffer(1024);
        gf::SocketDataResult result = c.socket.recvRawBytes({buffer.data(), buffer.size()});

        if (result.status == gf::SocketStatus::Data) {
            for (size_t j = 0; j < result.length; ++j) {
                char dir = buffer[j];

                float newX = c.state.x;
                float newY = c.state.y;

                switch (dir) {
                    case 'U': newY -= 50; break;
                    case 'D': newY += 50; break;
                    case 'L': newX -= 50; break;
                    case 'R': newX += 50; break;
                }

                game.movePlayer(c.id, newX, newY);

                auto& gPlayer = game.getPlayerInfo(c.id);
                c.state.x = gPlayer.x;
                c.state.y = gPlayer.y;

                gf::Log::info("Client %d moved %c -> position=(%.1f, %.1f)\n",
                            c.id, dir, c.state.x, c.state.y);
            }
        }

        else if (result.status == gf::SocketStatus::Close || result.status == gf::SocketStatus::Error) {
            toRemove.push_back(i);
        }
    }

    removeDisconnectedClients(toRemove);
    broadcastStates();
}

void GameNetworkServer::removeDisconnectedClients(const std::vector<size_t>& toRemove) {
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        selector.removeSocket(clients[*it].socket);
        gf::Log::info("Client %d déconnecté\n", clients[*it].id);
        clients.erase(clients.begin() + *it);
    }
}

void GameNetworkServer::broadcastStates() {
    for (auto& c : clients) {
        std::vector<uint8_t> buffer;
        for (auto& other : clients) {
            serialize(other.state, buffer);
        }
        c.socket.sendRawBytes({buffer.data(), buffer.size()});
        gf::Log::info("Envoi %zu octets au client %d\n", buffer.size(), c.id);
    }
}

void GameNetworkServer::plateauDisplayLoop() {
    while (true) {
        {
            std::lock_guard<std::mutex> lock(clientsMutex);

            std::vector<Plateau::PlayerInfo> infos;
            for (auto& c : clients) {
                infos.push_back({
                    static_cast<int>(c.state.x) / 50,
                    static_cast<int>(c.state.y) / 50,
                    static_cast<uint32_t>(c.id)
                });
            }

            game.getPlateau().printWithPlayers(infos);
        }

        std::cout << "----------------------" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}