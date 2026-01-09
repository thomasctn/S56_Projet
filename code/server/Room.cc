#include "Room.h"
#include "BotController.h"


Room::Room(uint32_t id, ServerNetwork& network)
: id(id)
, network(network)
{
    gf::Log::info("[Room %u] Créée\n", id);
}

void Room::addPlayer(uint32_t playerId) {
    if (players.size() >= MAX_PLAYERS) {
        gf::Log::warning("[Room %u] Room pleine, joueur %u non ajouté\n", id, playerId);
        return;
    }

    players.insert(playerId);
    gf::Log::info("[Room %u] Joueur %u ajouté\n", id, playerId);

    // Envoyer la confirmation au joueur qui a rejoint
    gf::Packet joinPacket;
    joinPacket.is(ServerJoinRoom{});
    network.send(playerId, joinPacket);

    // Broadcast de la liste des joueurs à tous
    ServerListRoomPlayers list;
    for (uint32_t pid : players)
        list.players.push_back(getPlayerData(pid));

    gf::Packet listPacket;
    listPacket.is(list);
    for (uint32_t pid : players)
        network.send(pid, listPacket);

    // Démarrage automatique si la room est pleine et que la partie n'existe pas encore
    if (!game && players.size() == MAX_PLAYERS) {
        startGame();
    }
}


void Room::removePlayer(uint32_t playerId) {
    players.erase(playerId);
    gf::Log::info("[Room %u] Joueur %u retiré\n", id, playerId);

    // Envoyer confirmation au joueur qui quitte
    gf::Packet leavePacket;
    leavePacket.is(ServerLeaveRoom{});
    network.send(playerId, leavePacket);

    // Broadcast de la liste mise à jour aux autres joueurs
    ServerListRoomPlayers list;
    for (uint32_t pid : players)
        list.players.push_back(getPlayerData(pid));

    gf::Packet listPacket;
    listPacket.is(list);
    for (uint32_t pid : players)
        network.send(pid, listPacket);

    // Si le joueur était dans la partie en cours, le retirer également
    if (game) {
        game->removePlayer(playerId);
    }

}


void Room::startGame() {
    if (game)
        return;

    constexpr int boardWidth = 27;
    constexpr int boardHeight = 27;

    game = std::make_unique<Game>(boardWidth, boardHeight);
    game->setRoom(*this);
    botManager = std::make_unique<BotManager>(*game, inputQueue);
    game->setBotManager(botManager.get());

    // --- Ajouter les joueurs ---
    size_t i = 0;
    for (uint32_t playerId : players) {
        if (game->getPlayers().find(playerId) == game->getPlayers().end()) {
            PlayerRole role = (i == 0) ? PlayerRole::PacMan : PlayerRole::Ghost;
            game->addPlayer(playerId, 100.0f, 100.0f, role);
        }
        ++i;
    }

    // --- Ajouter les bots ---
    int humans = players.size();
    int botsToAdd = NB_BOTS;    //MAX_PLAYERS - humans;

    for (int b = 0; b < botsToAdd; ++b) {
        uint32_t botId = generateBotId();
        game->addPlayer(botId, 0.f, 0.f, PlayerRole::Ghost);
        Player& bot = game->getPlayerInfo(botId);

        // Fournir l'ID au constructeur
        bot.controller = new BotController(botId);

        botManager->registerBot(botId);
    }




    // --- Spawner tous les joueurs ---
    for (auto& [id, playerPtr] : game->getPlayers()) {
        game->spawnPlayer(*playerPtr);
    }

    // Broadcast de ServerGameStart à tous les joueurs avec la carte et l'état initial
    ServerGameStart startMsg;
    startMsg.board = game->getBoard().toCommonData();
    for (auto& [id, playerPtr] : game->getPlayers()) {
        startMsg.players.push_back(playerPtr->getState());
    }

    gf::Packet startPacket;
    startPacket.is(startMsg);
    for (uint32_t pid : players)
        network.send(pid, startPacket);


    // Démarrer la boucle de jeu
    game->startGameLoop(50, inputQueue, network);

    gf::Log::info("[Room %u] Partie démarrée avec %zu joueurs\n", id, players.size());
}








void Room::handlePacket(PacketContext& ctx) {
    gf::Log::info("[Room %u] Paquet reçu joueur=%u type=%llu\n", 
                  id, ctx.senderId, static_cast<unsigned long long>(ctx.packet.getType()));

    // Pour ClientJoinRoom, on ne vérifie pas encore si le joueur est dans la room
    if (ctx.packet.getType() != ClientJoinRoom::type &&
        players.find(ctx.senderId) == players.end()) {
        gf::Log::warning("[Room %u] Joueur %u non autorisé\n", id, ctx.senderId);
        return;
    }

    switch (ctx.packet.getType()) {
        case ClientJoinRoom::type:
            handleClientJoinRoom(ctx);
            break;
        case ClientMove::type:
            handleClientMove(ctx);
            break;
        case ClientReady::type:
            handleClientReady(ctx);
            break;
        case ClientChangeRole::type:
            handleClientChangeRole(ctx);
            break;
        default:
            gf::Log::info("[Room %u] Paquet non traité (type=%llu)\n", 
                          id, static_cast<unsigned long long>(ctx.packet.getType()));
            break;
    }
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
}

void Room::handleClientReady(PacketContext& ctx) {
    if (!game)
        return;

    auto& player = game->getPlayerInfo(ctx.senderId);
    player.ready = true;
    gf::Log::info("[Room %u] Joueur %u ready\n", id, ctx.senderId);

    // Broadcast ServerReady à tous les joueurs
    ServerReady readyMsg;
    gf::Packet readyPacket;
    readyPacket.is(readyMsg);

    for (uint32_t pid : players)
        network.send(pid, readyPacket);


    // Si tous prêts, démarrer la partie
    if (allPlayersReady()) {
        startGame();
    }
}

void Room::handleClientChangeRole(PacketContext& ctx) {
    auto& player = game->getPlayerInfo(ctx.senderId);
    player.role = (player.role == PlayerRole::PacMan) ? PlayerRole::Ghost : PlayerRole::PacMan;

    // Broadcast le changement
    ServerChangeRole msg;
    gf::Packet rolePacket;
    rolePacket.is(msg);

    for (uint32_t pid : players)
        network.send(pid, rolePacket);

}

void Room::handleClientJoinRoom(PacketContext& ctx) {
    auto data = ctx.packet.as<ClientJoinRoom>();
    // Ici, tu peux vérifier que ctx.senderId veut bien rejoindre cette room
    if (data.room != id) {
        gf::Log::warning("[Room %u] Joueur %u tente de rejoindre une autre room %llu\n",
                         id, ctx.senderId, static_cast<unsigned long long>(data.room));
        return;
    }

    gf::Log::info("[Room %u] Joueur %u rejoint la room via ClientJoinRoom\n", id, ctx.senderId);
    addPlayer(ctx.senderId);
}


PlayerData Room::getPlayerData(uint32_t playerId) const {
    if (!game) return {};
    const auto& player = game->getPlayerInfo(playerId);
    return player.getState();
}

bool Room::allPlayersReady() const {
    if (!game) return false;
    for (const auto& [id, playerPtr] : game->getPlayers()) {
        if (!playerPtr->getState().ready)
            return false;
    }
    return true;
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

uint32_t Room::generateBotId() {
    static uint32_t nextBotId = 10000; // commence après les IDs joueurs
    return nextBotId++;
}
