#include "GameNetworkServer.h"
#include "../common/Serializable.h"
#include <gf/Log.h>
#include <thread>
#include <chrono>

GameNetworkServer::GameNetworkServer()
: listener("5000")
, nextId(1)
{
    if (!listener) {
        gf::Log::error("Impossible de démarrer le serveur !");
    }
    selector.addSocket(listener);
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
                switch (dir) {
                    case 'U': c.state.y -= 50; break;
                    case 'D': c.state.y += 50; break;
                    case 'L': c.state.x -= 50; break;
                    case 'R': c.state.x += 50; break;
                }
                gf::Log::info("Client %d moved %c -> position=(%.1f, %.1f)\n", c.id, dir, c.state.x, c.state.y);
            }
        } else if (result.status == gf::SocketStatus::Close || result.status == gf::SocketStatus::Error) {
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
