#include "Room.h"

Room::Room(uint32_t id, ServerNetwork& network)
: id(id)
, network(network)
{
    gf::Log::info("[Room %u] Créée\n", id);
}

void Room::addPlayer(uint32_t playerId) {
    players.insert(playerId);
    gf::Log::info("[Room %u] Joueur %u ajouté\n", id, playerId);

    if (!game) {
        startGame();
    } else {
        if (game->getPlayers().find(playerId) == game->getPlayers().end()) {
            PlayerRole role = PlayerRole::Ghost;
            game->addPlayer(playerId, 50.0f, 50.0f, role);
            game->spawnPlayer(game->getPlayerInfo(playerId));
        }
    }
}


void Room::removePlayer(uint32_t playerId) {
    players.erase(playerId);
    gf::Log::info("[Room %u] Joueur %u retiré\n", id, playerId);
}

void Room::startGame() {
    if (game)
        return;

    if (!game) {
        constexpr int boardWidth = 27;
        constexpr int boardHeight = 27;
        game = std::make_unique<Game>(boardWidth, boardHeight);
    }

    game->setRoom(*this);

    size_t i = 0;
    for (uint32_t playerId : players) {
        if (game->getPlayers().find(playerId) == game->getPlayers().end()) {
            PlayerRole role = (i == 0) ? PlayerRole::PacMan : PlayerRole::Ghost;
            game->addPlayer(playerId, 100.0f, 100.0f, role);
        }
        ++i;
    }


    for (auto& [id, playerPtr] : game->getPlayers()) {
        game->spawnPlayer(*playerPtr);
    }

    game->startGameLoop(50, inputQueue, network);

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
    gf::Log::info("[Room %u] Input ajouté joueur=%u dir=%c\n", id, ctx.senderId, data.moveDir);

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
    if (!game)
        return;

    auto& playersMap = game->getPlayers();

    GameState gs;
    gs.board = game->getBoard().toCommonData();

    for (auto& [id, playerPtr] : playersMap) {
        gs.clientStates.push_back(playerPtr->getState());
    }

    std::sort(
        gs.clientStates.begin(),
        gs.clientStates.end(),
        [](const PlayerData& a, const PlayerData& b) {
            return a.id < b.id;
        }
    );

    gf::Packet packet;
    packet.is(gs);

    for (uint32_t pid : players) {
        network.send(pid, packet);
    }
}

