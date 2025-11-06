#include <gf/TcpListener.h>
#include <gf/TcpSocket.h>
#include <iostream>

int main() {
    gf::TcpListener listener("5000"); // écoute sur le port 5000

    std::cout << "Serveur en attente..." << std::endl;

    gf::TcpSocket client = listener.accept();

    if (client) {
        std::cout << "Client connecté !" << std::endl;
    }

    return 0;
}
