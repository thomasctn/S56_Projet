#include "GameNetworkServer.h"


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
    game.startGameLoop(50, inputQueue);
    while (running) {
        if (selector.wait(gf::milliseconds(SPEED)) == gf::v1::SocketSelectorStatus::Event) {
            handleNewClient();
            handleClientData();
        }
    }
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

    uint32_t id = nextId++;

    game.getPlayers().emplace(id, std::make_unique<Player>(id));
    auto& newPlayer = *game.getPlayers()[id];
    newPlayer.socket = std::move(clientSocket);
    newPlayer.x = 100.0f;
    newPlayer.y = 100.0f;
    newPlayer.color = 0xFF0000FF;

    selector.addSocket(newPlayer.socket);
    if (id==1){
        newPlayer.setRole(PlayerRole::PacMan);
    }
    else{
        newPlayer.setRole(PlayerRole::Ghost);
    }
    game.spawnPlayer(newPlayer);



    gf::Log::info("Client connecté, id=%d\n", id);


}


void GameNetworkServer::handleClientData() {
    std::vector<uint32_t> toRemove;

    std::lock_guard<std::mutex> lock(playersMutex);
    auto& players = game.getPlayers();

    for (auto& [id, playerPtr] : players) {
        Player& p = *playerPtr;

        if (!selector.isReady(p.socket))
            continue;

        gf::Packet packet;
        switch (p.socket.recvPacket(packet)) {
            case gf::SocketStatus::Data: {
                if (packet.getType() == ClientMove::type) {
                    auto data = packet.as<ClientMove>();
                    Direction dir;
                    switch (data.moveDir) {
                        case 'U': dir = Direction::Up; break;
                        case 'D': dir = Direction::Down; break;
                        case 'L': dir = Direction::Left; break;
                        case 'R': dir = Direction::Right; break;
                        default: continue;
                    }

                    inputQueue.push({id, dir});
                }
                break;
            }

            case gf::SocketStatus::Error:
            case gf::SocketStatus::Close:
                toRemove.push_back(id);
                break;

            case gf::SocketStatus::Block:
                break;
        }
    }

    removeDisconnectedPlayers(toRemove);
    broadcastStates();
}



void GameNetworkServer::removeDisconnectedPlayers(const std::vector<uint32_t>& disconnectedIds) {
    auto& players = game.getPlayers();

    for (uint32_t id : disconnectedIds) {
        auto it = players.find(id);
        if (it == players.end())
            continue;

        selector.removeSocket(it->second->socket);
        gf::Log::info("Client %d déconnecté\n", id);

        players.erase(it);
    }
}


void GameNetworkServer::broadcastStates()
{
    auto& players = game.getPlayers();

    for (auto& [id, playerPtr] : players)
    {
        std::vector<PlayerData> states;

        for (auto& [otherId, otherPtr] : players) {
            states.push_back(otherPtr->getState());
        }

        std::sort(states.begin(), states.end(), [](const PlayerData &a, const PlayerData &b){
            return a.id < b.id;
        });

        GameState gs;
        gs.clientStates = states;
        gs.board = game.getBoard().toCommonData();

        gf::Packet packet;
        packet.is(gs);

        playerPtr->socket.sendPacket(packet);
    }
}


Game& GameNetworkServer::getGame() {
    return game;
}

std::mutex& GameNetworkServer::getPlayersMutex() {
    return playersMutex;
}
