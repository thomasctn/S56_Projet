#include <gf/TcpListener.h>
#include <gf/TcpSocket.h>
#include <gf/SocketSelector.h>
#include <gf/Packet.h>
#include <gf/Log.h>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include "../common/Types.h"

struct ClientInfo {
    uint32_t id;
    gf::TcpSocket socket;
    ClientState state;
};

int main() {
    gf::Log::info("Démarrage du serveur...");

    gf::TcpListener listener("5000");
    if (!listener) {
        gf::Log::error("Impossible de démarrer le serveur !");
        return -1;
    }

    gf::SocketSelector selector;
    selector.addSocket(listener);

    std::vector<ClientInfo> clients;
    std::mutex clientsMutex;
    uint32_t nextId = 1;

    while (true) {
        // attendre jusqu'à un événement (nouveau client ou données)
        if (selector.wait(gf::milliseconds(100)) == gf::v1::SocketSelectorStatus::Event) {

            // nouveau client
            if (selector.isReady(listener)) {
                gf::TcpSocket clientSocket = listener.accept();
                if (clientSocket) {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    ClientInfo ci;
                    ci.id = nextId++;
                    ci.socket = std::move(clientSocket);
                    ci.state = {ci.id, 100.0f, 100.0f, 0xFF0000FF};
                    clients.push_back(std::move(ci));
                    gf::Log::info("Client connecté, id=%d\n", clients.back().id);
                    selector.addSocket(clients.back().socket);
                }
            }

            // traiter les clients
            std::vector<size_t> toRemove;
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                for (size_t i = 0; i < clients.size(); ++i) {
                    auto& c = clients[i];

                    // vérifier si des données sont prêtes
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
                    } else if (result.status == gf::SocketStatus::Close) {
                        gf::Log::info("Client %d déconnecté\n", c.id);
                        toRemove.push_back(i);
                    } else if (result.status == gf::SocketStatus::Error) {
                        gf::Log::error("Erreur lors de la réception pour le client %d\n", c.id);
                        toRemove.push_back(i);
                    }
                }

                // supprimer les clients déconnectés
                for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
                    selector.removeSocket(clients[*it].socket);
                    clients.erase(clients.begin() + *it);
                }
            }

            // envoyer les états à tous les clients
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                for (auto& c : clients) {
                    std::vector<uint8_t> buffer;
                    for (auto& other : clients) {
                        serialize(other.state, buffer);
                    }
                    c.socket.sendRawBytes({buffer.data(), buffer.size()});
                    gf::Log::info("Envoi %zu octets au client %d\n", buffer.size(), c.id);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return 0;
}
