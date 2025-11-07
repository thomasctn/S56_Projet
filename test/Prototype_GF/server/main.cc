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
    gf::Log::info("Démarrage du serveur...\n");

    gf::TcpListener listener("5000");
    if (!listener) {
        gf::Log::error("Impossible de démarrer le serveur !\n");
        return -1;
    }

    gf::SocketSelector selector;
    selector.addSocket(listener);

    std::vector<ClientInfo> clients;
    std::mutex clientsMutex;
    uint32_t nextId = 1;

    while (true) {
        if (selector.wait(gf::milliseconds(100)) == gf::v1::SocketSelectorStatus::Event) {
            if (selector.isReady(listener)) {
                // nouveau client
                gf::TcpSocket clientSocket = listener.accept();
                if (clientSocket) {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    ClientInfo ci;
                    ci.id = nextId++;
                    ci.socket = std::move(clientSocket);
                    ci.state = {ci.id, 100.0f, 100.0f, 0xFFFFFFFF}; // couleur par défaut
                    clients.push_back(std::move(ci));
                    gf::Log::info("Client connecté, id=%d\n", clients.back().id);

                    // envoyer l'ID au client
                    uint32_t idToSend = clients.back().id;
                    clients.back().socket.sendRawBytes({reinterpret_cast<uint8_t*>(&idToSend), sizeof(idToSend)});
                }
            }

            // traiter les clients
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (auto& c : clients) {
                std::vector<uint8_t> buffer(1024);
                gf::SocketDataResult result = c.socket.recvRawBytes({buffer.data(), buffer.size()});

                if (result.status == gf::SocketStatus::Data) {
                    for (size_t i = 0; i < result.length; ++i) {
                        char dir = buffer[i];
                        switch (dir) {
                            case 'U': c.state.y -= 1; break; // déplacement plus fin
                            case 'D': c.state.y += 1; break;
                            case 'L': c.state.x -= 1; break;
                            case 'R': c.state.x += 1; break;
                        }
                        gf::Log::info("Client %d moved %c -> position=(%.1f, %.1f)", c.id, dir, c.state.x, c.state.y);
                    }
                } else if (result.status == gf::SocketStatus::Close) {
                    gf::Log::info("Client %d déconnecté\n", c.id);
                } else if (result.status == gf::SocketStatus::Error) {
                    gf::Log::error("Erreur lors de la réception pour le client %d\n", c.id);
                }
            }

            // envoyer les états à tous
            std::lock_guard<std::mutex> lock2(clientsMutex);
            for (auto& c : clients) {
                std::vector<uint8_t> buffer;
                for (auto& other : clients) {
                    serialize(other.state, buffer);
                }
                c.socket.sendRawBytes({buffer.data(), buffer.size()});
            }
        }
    }

    return 0;
}
