#include <gf/TcpSocket.h>
#include <iostream>

int main() {
    gf::TcpSocket socket("127.0.0.1", "5000");

    if (socket) {
        std::cout << "Connecté au serveur !" << std::endl;
    }

    return 0;
}
