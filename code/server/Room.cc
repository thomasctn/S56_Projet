#include "Room.h"

Room::Room(uint32_t id, ServerNetwork& network)
: id(id)
, network(network)
, gameStarted(false)
{
    gf::Log::info("[Room %u] Créée\n", id);
}

void Room::addPlayer(uint32_t playerId) {
    players.insert(playerId);
    gf::Log::info("[Room %u] Joueur %u ajouté\n", id, playerId);

    if (!gameStarted && players.size() >= 1) {
        startGame();
    }
}

void Room::removePlayer(uint32_t playerId) {
    players.erase(playerId);
    gf::Log::info("[Room %u] Joueur %u retiré\n", id, playerId);
}

void Room::startGame() {
    if (gameStarted)
        return;

    if (!game) {
        constexpr int boardWidth = 10;
        constexpr int boardHeight = 10;
        game = std::make_unique<Game>(boardWidth, boardHeight);
    }

    game->setRoom(*this);

    for (uint32_t playerId : players) {
        if (game->getPlayers().find(playerId) == game->getPlayers().end()) {
            game->addPlayer(playerId, 50.0f, 50.0f); 
        }
    }

    for (auto& [id, playerPtr] : game->getPlayers()) {
        game->spawnPlayer(*playerPtr);
    }

    game->startGameLoop(50, inputQueue, network);

    gameStarted = true;

    gf::Log::info("[Room %u] Partie démarrée avec %zu joueurs\n", id, players.size());
}







void Room::handlePacket(PacketContext& ctx) {
gf::Log::info("[Room %u] Paquet reçu joueur=%u type=%llu\n", 
              id, 
              ctx.senderId, 
              static_cast<unsigned long long>(ctx.packet.getType()));


    if (players.find(ctx.senderId) == players.end()) {
        gf::Log::warning(
            "[Room %u] Joueur %u non autorisé\n",
            id,
            ctx.senderId
        );
        return;
    }

    if (ctx.packet.getType() == ClientMove::type) {
        handleClientMove(ctx);
        return;
    }

gf::Log::info(
    "[Room %u] Paquet non traité (type=%llu)\n",
    id,
    static_cast<unsigned long long>(ctx.packet.getType())
);

}

void Room::handleClientMove(PacketContext& ctx) {
    auto data = ctx.packet.as<ClientMove>();

    Direction dir;
    switch (data.moveDir) {
        case 'U': dir = Direction::Up; break;
        case 'D': dir = Direction::Down; break;
        case 'L': dir = Direction::Left; break;
        case 'R': dir = Direction::Right; break;
        default:
            gf::Log::warning("[Room %u] Direction invalide\n", id);
            return;
    }

    inputQueue.push({ctx.senderId, dir});

    gf::Log::info(
        "[Room %u] Input ajouté joueur=%u dir=%c\n",
        id,
        ctx.senderId,
        data.moveDir
    );
}

void Room::broadcastState() {
    if (!gameStarted)
        return;

    gf::Log::info("[Room %u] Broadcast de l'état du jeu\n", id);

    GameState state;
    state.board = game->getBoard().toCommonData();

    for (auto& [id, player] : game->getPlayers()) {
        state.clientStates.push_back(player->getState());
    }


    gf::Packet packet;
    packet.is(state);

    for (uint32_t pid : players) {
        network.send(pid, packet);
    }
}
