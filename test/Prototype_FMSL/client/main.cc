#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <map>
#include <cstdint>
#include <vector>
#include <chrono>
#include <thread>

struct Player {
    float x, y;
};

sf::Color getColorById(uint32_t id) {
    static std::vector<sf::Color> colors = {
        sf::Color::Yellow, sf::Color::Red, sf::Color::Green,
        sf::Color::Blue, sf::Color::Magenta, sf::Color::Cyan
    };
    return colors[id % colors.size()];
}

int main() {
    sf::TcpSocket socket;
    if (socket.connect("127.0.0.1", 5000) != sf::Socket::Done) {
        std::cout << "[CLIENT] Impossible de se connecter au serveur" << std::endl;
        return 1;
    }
    socket.setBlocking(false);

    sf::RenderWindow window(sf::VideoMode(800, 600), "Client");

    uint32_t myId = 0;
    std::map<uint32_t, Player> players;

    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // -------------------------
        // Événements uniquement sur la fenêtre active
        // -------------------------
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // -------------------------
        // Calcul direction
        // -------------------------
        float dx = 0.f, dy = 0.f;
        if (window.hasFocus()) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  dx = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dx = 1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    dy = -1.f;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  dy = 1.f;
        }

        // -------------------------
        // Envoyer direction toutes les 0,1s
        // -------------------------
        static auto lastSend = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<float>(now - lastSend).count() >= 0.1f) {
            lastSend = now;
            sf::Packet packet;
            packet << dx << dy;
            socket.send(packet);
        }

        // -------------------------
        // Recevoir positions
        // -------------------------
        sf::Packet recv;
        while (socket.receive(recv) == sf::Socket::Done) {
            uint32_t count;
            if (!(recv >> count)) break;

            for (uint32_t i = 0; i < count; ++i) {
                uint32_t id;
                float x, y;
                if (!(recv >> id >> x >> y)) continue;

                if (myId == 0) myId = id;
                players[id] = Player{x, y};
            }
        }

        // -------------------------
        // Rendu
        // -------------------------
        window.clear(sf::Color::Black);

        for (auto& [id, p] : players) {
            sf::RectangleShape rect(sf::Vector2f(20.f, 20.f));
            rect.setFillColor(getColorById(id));
            rect.setPosition(p.x, p.y);
            window.draw(rect);
        }

        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}
