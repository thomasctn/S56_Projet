#include "ClientNetworkHandler.h"
#include <gf/Log.h>



std::thread startNetworkReceiver(
    gf::TcpSocket& socket,
    std::atomic<bool>& running,
    std::queue<gf::Packet>& packetQueue,
    std::mutex& packetMutex
) {
    return std::thread([&]() {
        while (running.load()) {
            gf::Packet packet;

            switch (socket.recvPacket(packet)) {
                case gf::SocketStatus::Data: {
                    std::lock_guard<std::mutex> lock(packetMutex);
                    packetQueue.push(std::move(packet));
                    break;
                }

                case gf::SocketStatus::Close:
                    gf::Log::info("Serveur déconnecté\n");
                    running.store(false);
                    break;

                case gf::SocketStatus::Error:
                    if (running.load()) {
                        gf::Log::error("Erreur réseau côté client\n");
                    }
                    break;

                default:
                    break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
}