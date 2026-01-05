#include "GameNetworkServer.h"

#define SPEED 30

GameNetworkServer::GameNetworkServer()
    : listener("5000"), nextId(1), game(27, 27), running(true)
{
    if (!listener)
        gf::Log::error("Impossible de démarrer le serveur !");
    
    selector.addSocket(listener);
}

int GameNetworkServer::run()
{
    gf::Log::info("Serveur démarré !");
    game.startGameLoop(50);
    while (running) {
        if (selector.wait(gf::milliseconds(SPEED)) == gf::v1::SocketSelectorStatus::Event) {
            handleNewClient();
            handleClientData();
        }
    }
    game.startGameLoop();
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

    std::lock_guard<std::mutex> lock(playersMutex);

    Player newPlayer(nextId++);
    newPlayer.socket = std::move(clientSocket);
    newPlayer.x = 100.0f;
    newPlayer.y = 100.0f;
    newPlayer.color = 0xFF0000FF;

    selector.addSocket(newPlayer.socket);
    game.getPlayers().push_back(std::move(newPlayer));

    gf::Log::info("Client connecté, id=%d\n", game.getPlayers().back().id);
}


void GameNetworkServer::handleClientData() {
    std::vector<size_t> toRemove;

    std::lock_guard<std::mutex> lock(playersMutex);
    auto& players = game.getPlayers(); // accès centralisé

    for (size_t i = 0; i < players.size(); ++i) {
        auto& p = players[i];

        if (!selector.isReady(p.socket))
            continue;

        gf::Packet packet;
        switch (p.socket.recvPacket(packet)) {
            case gf::SocketStatus::Data: {
                if (packet.getType() == ClientMove::type) {
                    auto data = packet.as<ClientMove>();
                    Direction direction;
                    switch (data.moveDir) {
                        case 'U': direction = Direction::Up; break;
                        case 'D': direction = Direction::Down; break;
                        case 'L': direction = Direction::Left; break;
                        case 'R': direction = Direction::Right; break;
                        default: continue;
                    }
                    game.requestMove(p.id, direction);

                    // mettre à jour l'état si nécessaire
                    gf::Log::info("Client %d moved %c -> position=(%.1f, %.1f)\n",
                        p.id, data.moveDir, p.x, p.y);
                }
                break;
            }

            case gf::SocketStatus::Error:
            case gf::SocketStatus::Close:
                toRemove.push_back(i);
                break;

            case gf::SocketStatus::Block:
                break;
        }
    }

    removeDisconnectedPlayers(toRemove);
    broadcastStates();
}


void GameNetworkServer::removeDisconnectedPlayers(const std::vector<size_t> &toRemove) {
    auto& players = game.getPlayers();
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it)
    {
        selector.removeSocket(players[*it].socket);
        gf::Log::info("Client %d déconnecté\n", players[*it].id);
        players.erase(players.begin() + *it);
    }
}

void GameNetworkServer::broadcastStates()
{
    auto& players = game.getPlayers();

    for (auto& p : players)
    {
        std::vector<ClientState> states;
        for (auto& other : players)
            states.push_back(other.getState());

        GameState gs;
        gs.clientStates = states;
        gs.bd = game.getPlateau().toData();
        gf::Packet packet;
        packet.is(gs);
        p.socket.sendPacket(packet);
    }


    /* 
    TODO
    PLATEAU A ENVOYER
    const Plateau& p = game.getPlateau();
    */
    }

Game& GameNetworkServer::getGame() {
    return game;
}

std::mutex& GameNetworkServer::getPlayersMutex() {
    return playersMutex;
}
