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
    /*
    if (!game && players.size() == MAX_PLAYERS) {
        startGame();
    }
    */
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
    int botsToAdd = NB_BOTS; // ou MAX_PLAYERS - humans;

    for (int b = 0; b < botsToAdd; ++b) {
        uint32_t botId = generateBotId();
        game->addPlayer(botId, 0.f, 0.f, PlayerRole::Ghost);
        Player& bot = game->getPlayerInfo(botId);

        bot.isBot = true; // <- important
        bot.controller = new BotController(botId);

        botManager->registerBot(botId);
    }

    // Générer le graphe pour tous les bots
    for (auto& [id, playerPtr] : game->getPlayers()) {
        if (playerPtr->isBot) {
            auto* botController = dynamic_cast<BotController*>(playerPtr->controller);
            if (botController) {
                botController->generateGraph(game->getBoard());
                gf::Log::info("[Room] Graphe généré pour le bot %u\n", id);
            }
        }
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

    if (players.find(ctx.senderId) == players.end()) {
        gf::Log::warning("[Room %u] Joueur %u non autorisé\n", id, ctx.senderId);
        return;
    }

    switch (ctx.packet.getType()) {
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
    // Convertir le packet en ClientReady
    auto data = ctx.packet.as<ClientReady>();

    gf::Log::info("[Room %u] Joueur %u ready=%s\n", id, ctx.senderId, data.ready ? "true" : "false");

    // --- Si le Game existe, mettre à jour le Player ---
    if (game) {
        auto& player = game->getPlayerInfo(ctx.senderId);
        player.ready = data.ready;
    } else {
        preGameReady[ctx.senderId] = data.ready;
    }

    // Broadcast ServerReady à tous les joueurs
    ServerReady readyMsg;
    gf::Packet readyPacket;
    readyPacket.is(readyMsg);
    for (uint32_t pid : players)
        network.send(pid, readyPacket);

    // Vérifier si tous les joueurs sont prêts
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

PlayerData Room::getPlayerData(uint32_t playerId) const {
    if (!game) return {};
    const auto& player = game->getPlayerInfo(playerId);
    return player.getState();
}

bool Room::allPlayersReady() const {
    for (uint32_t pid : players) {
        bool isReady = false;
        if (game) {
            const Player& p = game->getPlayerInfo(pid);
            isReady = p.ready;
        } else {
            auto it = preGameReady.find(pid);
            if (it != preGameReady.end())
                isReady = it->second;
        }

        if (!isReady)
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
