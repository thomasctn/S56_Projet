#include "Room.h"
#include "BotController.h"


Room::Room(uint32_t id, ServerNetwork& network)
: id(id)
, network(network)
{
    gf::Log::info("[Room %u] Créée\n", id);
}

void Room::addPlayer(uint32_t playerId) {
    if (players.size() >= settings.roomSize) {
        gf::Log::warning("[Room %u] Room pleine (%zu / %u), joueur %u refusé\n",
                         id, players.size(), settings.roomSize, playerId);
        return;
    }

    players.insert(playerId);
    gf::Log::info("[Room %u] Joueur %u ajouté\n", id, playerId);

    // Pré‑assigner un rôle si game n'existe pas
    if (!game) {
        // Choisir un rôle par défaut : 1er PacMan, le reste Ghost
        PlayerRole role = PlayerRole::Ghost;
        bool hasPacMan = false;
        for (auto& [_, r] : preGameRoles)
            if (r == PlayerRole::PacMan)
                hasPacMan = true;

        if (!hasPacMan)
            role = PlayerRole::PacMan;

        preGameRoles[playerId] = role;
        gf::Log::info("[Room %u] Joueur %u rôle pré-game assigné : %s\n",
                      id, playerId,
                      (role == PlayerRole::PacMan ? "PacMan" : "Ghost"));
    }

    gf::Packet joinPacket;
    joinPacket.is(ServerJoinRoom{});
    network.send(playerId, joinPacket);

    broadcastRoomPlayers();
    broadcastRoomSettings();
}



void Room::removePlayer(uint32_t playerId) {
    players.erase(playerId);
    gf::Log::info("[Room %u] Joueur %u retiré\n", id, playerId);

    gf::Packet leavePacket;
    leavePacket.is(ServerLeaveRoom{});
    network.send(playerId, leavePacket);

    broadcastRoomPlayers();
    broadcastRoomSettings();

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
            PlayerRole role = PlayerRole::Spectator;
            auto it = preGameRoles.find(playerId);
            if (it != preGameRoles.end())
                role = it->second;

            game->addPlayer(playerId, 100.0f, 100.0f, role);
        }
    }



    // --- Ajouter les bots ---
    int humans = players.size();
    int botsToAdd = settings.nbBot;

    for (int b = 0; b < botsToAdd; ++b) {
        uint32_t botId = generateBotId();

        // Ajouter le joueur bot dans le Game
        game->addPlayer(botId, 0.f, 0.f, PlayerRole::Ghost);
        Player& bot = game->getPlayerInfo(botId);

        bot.isBot = true; // important

        // Créer le BotController et l'enregistrer dans BotManager
        auto botCtrl = std::make_unique<BotController>(botId);
        botManager->registerBot(botId, std::move(botCtrl));
    }

    // --- Générer le graphe global pour tous les bots ---
    botManager->generateGraph(game->getBoard());
    gf::Log::info("[Room] Graphe global généré pour tous les bots\n");






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
    startMsg.pacgommes = game->getBoard().getPacgommes();
    gf::Packet startPacket;
    startPacket.is(startMsg);
    for (uint32_t pid : players)
        network.send(pid, startPacket);


    // Démarrer la boucle de jeu
    game->startGameLoop(50, inputQueue, network);

    gf::Log::info("[Room %u] Partie démarrée avec %zu joueurs\n", id, players.size());
}








void Room::handlePacket(PacketContext& ctx) {
    //gf::Log::info("[Room %u] Paquet reçu joueur=%u type=%llu\n",id, ctx.senderId, static_cast<unsigned long long>(ctx.packet.getType()));

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
        case ClientChangeRoomCharacterData::type:
            handleClientChange(ctx);
            break;
        case ClientChangeRoomSettings::type:
            handleClientChangeRoomSettings(ctx);
            break;

        default:
            gf::Log::info("[Room %u] Paquet non traité (type=%llu)\n", 
                          id, static_cast<unsigned long long>(ctx.packet.getType()));
            break;
    }
}



void Room::handleClientMove(PacketContext& ctx) {
    auto data = ctx.packet.as<ClientMove>();
    //gf::Log::info("[Room %u] Input ajouté joueur=%u dir=%c\n", id, ctx.senderId, data.moveDir);

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
    if (allPlayersReady() && (players.size() == settings.roomSize)) {
        startGame();
    }
}



void Room::handleClientChange(PacketContext& ctx) {
    if (game) {
        return;
    } else {
        auto it = preGameRoles.find(ctx.senderId);
        PlayerRole currentRole = (it != preGameRoles.end()) ? it->second : PlayerRole::PacMan;

        // Vérifier qu'au moins un PacMan reste
        if (currentRole == PlayerRole::PacMan) {
            bool otherPacManExists = false;
            for (auto& [id, r] : preGameRoles) {
                if (id != ctx.senderId && r == PlayerRole::PacMan) {
                    otherPacManExists = true;
                    break;
                }
            }
            if (!otherPacManExists) {
                gf::Log::info("[Room %u] Changement de rôle refusé pour joueur %u : au moins un PacMan requis\n",
                              id, ctx.senderId);
                return;
            }
        }

        PlayerRole newRole = (currentRole == PlayerRole::PacMan) ? PlayerRole::Ghost : PlayerRole::PacMan;
        preGameRoles[ctx.senderId] = newRole;

        gf::Log::info("[Room %u] Joueur %u choisit rôle pré-game : %s\n",
                      id, ctx.senderId,
                      (newRole == PlayerRole::PacMan ? "PacMan" : "Ghost"));
    }

    ServerChangeRoomCharacterData msg;
    gf::Packet answer;
    answer.is(msg);
    network.send(ctx.senderId, answer);
    broadcastRoomPlayers();
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

void Room::broadcastRoomPlayers()
{
    ServerListRoomPlayers list;

    for (uint32_t pid : players) {
        PlayerData pdata;

        if (game) {
            const Player& p = game->getPlayerInfo(pid);
            pdata = p.getState();
        } else {
            pdata.id = pid;
            pdata.role = preGameRoles.count(pid) ? preGameRoles.at(pid) : PlayerRole::Spectator;
            pdata.ready = preGameReady.count(pid) ? preGameReady.at(pid) : false;
        }

        list.players.push_back(pdata);
    }

    /*
    // --- Log pour debug ---
    gf::Log::info("[Room %u] BroadcastRoomPlayers : %zu joueurs\n", id, list.players.size());
    for (auto& p : list.players) {
        const char* roleStr = "Unknown";
        switch(p.role) {
            case PlayerRole::PacMan:    roleStr = "PacMan"; break;
            case PlayerRole::Ghost:     roleStr = "Ghost"; break;
            case PlayerRole::Spectator: roleStr = "Spectator"; break;
        }
        gf::Log::info(" -> Player %u : role=%s, ready=%s\n",
                      p.id,
                      roleStr,
                      p.ready ? "true" : "false");
    }
    */

    gf::Packet packet;
    packet.is(list);

    for (uint32_t pid : players) {
        network.send(pid, packet);
    }
}


void Room::broadcastRoomSettings()
{
    ServerRoomSettings msg;
    msg.settings = settings;
    gf::Log::info("[Room %u] Test %u\n",id, settings.roomSize);
    gf::Packet packet;
    packet.is(msg);

    for (uint32_t pid : players) {
        network.send(pid, packet);
    }
}

void Room::broadcastState() {
    if (!game)
        return;

    auto& playersMap = game->getPlayers();

    ServerGameState gs;
    gs.board = game->getBoard().toCommonData();
    gs.pacgommes = game->getBoard().getPacgommes();
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

    if (game->isGameStarted()) {
        unsigned int duration = 0;
        if (settings.roomSize > 0) {
            duration = settings.gameDuration; // durée de la partie depuis la room
        }
        int elapsed = static_cast<int>(game->getElapsedSeconds());
        gs.timeLeft = (elapsed >= duration) ? 0 : (duration - elapsed);
    } else {
        gs.timeLeft = settings.gameDuration;
    }

        //gf::Log::info("[Room %u] Mises à jour : temps : %u\n",id, gs.timeLeft);

    gf::Packet packet;
    packet.is(gs);

    for (uint32_t pid : players) {
        network.send(pid, packet);
    }
}

uint32_t Room::generateBotId() {
    static uint32_t nextBotId = 10000; // commence après les ID joueurs
    return nextBotId++;
}

void Room::setSettings(const RoomSettings& newSettings){
    if (game) {
        gf::Log::warning(
            "[Room %u] Changement de règles refusé : partie déjà lancée\n",
            id
        );
        return;
    }

    if (newSettings.roomSize < players.size()) {
        gf::Log::warning(
            "[Room %u] Nouvelle taille de room trop petite (%u) : %zu joueurs déjà présents\n",
            id,
            newSettings.roomSize,
            players.size()
        );
        broadcastRoomSettings();
        return;
    }

    if (newSettings.nbBot < 0) {
        gf::Log::warning("[Room %u] On ne peut pas avoir -1 bots\n",id);
        broadcastRoomSettings();
        return;
    }

    if ((newSettings.gameDuration < MIN_DURATION) || (newSettings.gameDuration > MAX_DURATION)) {
        gf::Log::warning("[Room %u] Les temps demander sont incorects\n",id);
        broadcastRoomSettings();
        return;
    }

    settings.roomSize = newSettings.roomSize;
    settings.nbBot = newSettings.nbBot;
    settings.gameDuration = newSettings.gameDuration;

    gf::Log::info(
        "[Room %u] Règles mises à jour : roomSize=%u nbBot=%u duration=%u\n",
        id,
        settings.roomSize,
        settings.nbBot,
        settings.gameDuration
    );

    // Broadcast à tous les clients
    broadcastRoomSettings();
}

void Room::handleClientChangeRoomSettings(PacketContext& ctx)
{
    auto data = ctx.packet.as<ClientChangeRoomSettings>();

    // (optionnel) limiter au host
    // if (ctx.senderId != *players.begin()) return;
        // Ack uniquement au client demandeur
    ServerChangeRoomSettings ack;
    gf::Packet packet;
    packet.is(ack);
    network.send(ctx.senderId, packet);
    setSettings(data.newSettings);
}

void Room::endGame(GameEndReason reason) {
    notifyGameEnded(reason);
    resetPlayersState();
    std::thread([this]() {
        cleanupGame();
    }).detach();
}



void Room::notifyGameEnded(GameEndReason reason) {
    gf::Log::info("[Room %u] Notification de fin de partie\n", id);

    ServerGameEnd msg;
    msg.reason = reason;
    gf::Packet packet;
    packet.is(msg);

    for (uint32_t pid : players) {
        network.send(pid, packet);
    }
}

void Room::cleanupGame() {
    if (!game) return;

    gf::Log::info("[Room %u] Nettoyage de la partie\n", id);

    game->stopGameLoop();
    game->joinGameLoop();

    botManager.reset();
    game.reset();
    preGameReady.clear();

    gf::Log::info("[Room %u] Nettoyage fini\n", id);

}




void Room::resetPlayersState() {
    gf::Log::info("[Room %u] Reset états de joueurs\n", id);
    for (uint32_t pid : players) {
        preGameReady[pid] = false;
    }
}

void Room::notifyGameEndedAsync(GameEndReason reason) {
    notifyGameEnded(reason);
    resetPlayersState();
    std::thread([this]() {
        cleanupGame();
    }).detach();
}


void Room::broadcastPreGame(unsigned int remaining)
{
    ServerGamePreStart msg;
    msg.timeLeft = static_cast<uint32_t>(remaining);
    //gf::Log::info("[Room %u] Broadcast PreGame time : %u\n",id,remaining);
    gf::Packet packet;
    packet.is(msg);

    for (uint32_t pid : players) {
        network.send(pid, packet);
    }
}
