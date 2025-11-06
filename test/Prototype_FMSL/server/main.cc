#include <SFML/Network.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>
#include <algorithm>

struct Player {
    float x = 100.f;
    float y = 100.f;
    uint32_t id;
};

int main() {
    sf::TcpListener listener;
    if (listener.listen(5000) != sf::Socket::Done) {
        std::cout << "[SERVER] Impossible de démarrer le serveur" << std::endl;
        return 1;
    }
    listener.setBlocking(false);

    std::vector<sf::TcpSocket*> clients;
    std::map<sf::TcpSocket*, Player> players;
    uint32_t nextId = 1;

    std::cout << "[SERVER] Serveur démarré sur le port 5000" << std::endl;

    auto lastBroadcast = std::chrono::steady_clock::now();
    auto lastUpdate = std::chrono::steady_clock::now();
    float speed = 200.f;

    while (true) {
        // -------------------------
        //Accepter nouveaux clients
        // -------------------------
        sf::TcpSocket* client = new sf::TcpSocket;
        if (listener.accept(*client) == sf::Socket::Done) {
            client->setBlocking(false);
            clients.push_back(client);

            Player p;
            p.id = nextId++;
            players[client] = p;

            std::cout << "[SERVER] Nouveau client connecté, ID=" << p.id << std::endl;

            // Envoi des positions actuelles au nouveau client
            sf::Packet initPacket;
            initPacket << static_cast<uint32_t>(players.size());
            for (auto& [s, pl] : players)
                initPacket << pl.id << pl.x << pl.y;
            client->send(initPacket);
        } else {
            delete client;
        }

        // -------------------------
        // Recevoir les directions
        // -------------------------
        std::vector<sf::TcpSocket*> disconnected;
        for (auto* sock : clients) {
            sf::Packet packet;
            sf::Socket::Status status = sock->receive(packet);

            if (status == sf::Socket::Done) {
                float dx, dy;
                packet >> dx >> dy;
                players[sock].x += dx * speed * 0.1f;
                players[sock].y += dy * speed * 0.1f;

                std::cout << "[SERVER] ID=" << players[sock].id
                          << " dx=" << dx << " dy=" << dy
                          << " => x=" << players[sock].x
                          << " y=" << players[sock].y << std::endl;

            } else if (status == sf::Socket::Disconnected) {
                disconnected.push_back(sock);
            }
        }

        // -------------------------
        // Supprimer les clients déconnectés
        // -------------------------
        for (auto* sock : disconnected) {
            std::cout << "[SERVER] Client ID=" << players[sock].id << " déconnecté" << std::endl;
            clients.erase(std::remove(clients.begin(), clients.end(), sock), clients.end());
            players.erase(sock);
            delete sock;
        }

        // -------------------------
        // Diffuser toutes les positions toutes les 0,05s
        // -------------------------
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<float>(now - lastBroadcast).count() >= 0.05f) {
            lastBroadcast = now;

            sf::Packet update;
            update << static_cast<uint32_t>(players.size());
            for (auto& [s, p] : players)
                update << p.id << p.x << p.y;

            for (auto* sock : clients) {
                sock->send(update);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
