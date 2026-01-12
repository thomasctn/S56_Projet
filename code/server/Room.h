#pragma once

#include <unordered_set>
#include <cstdint>
#include <memory>

#include <gf/Log.h>
#include <gf/Packet.h>

#include "ServerNetwork.h"
#include "Game.h"
#include "../common/Protocol.h"
#include "PacketContext.h"
#include "InputQueue.h"
#include "../common/Constants.h"
#include "BotController.h"
#include "BotManager.h" 


class Room {
public:
    Room(uint32_t id, ServerNetwork& network);

    void addPlayer(uint32_t playerId);
    void removePlayer(uint32_t playerId);

    void handlePacket(PacketContext& ctx);

    Game& getGame() { return *game; }

    std::unique_ptr<Game> game;
    uint32_t id;
    ServerNetwork& network;
    std::unordered_set<uint32_t> players;
    InputQueue inputQueue;

    Room* room = nullptr;
    void broadcastState();
    void broadcastRoomPlayers();
    void broadcastRoomSettings();


    bool allPlayersReady() const;
    PlayerData getPlayerData(uint32_t playerId) const;
    void handleClientChange(PacketContext& ctx);
    void handleClientReady(PacketContext& ctx);
    void handleClientMove(PacketContext& ctx);
    void handleClientChangeRoomSettings(PacketContext& ctx);

    const RoomSettings& getSettings() const { return settings; }
    void setSettings(const RoomSettings& newSettings);

    bool isFull() const {return players.size() >= settings.roomSize;}
    unsigned int getMaxPlayers() const {return settings.roomSize;}

private:
    void startGame();
    void setRoom(Room& r) { room = &r; }
    uint32_t generateBotId();

    std::unique_ptr<BotManager> botManager;
    std::unordered_map<uint32_t, bool> preGameReady;

    RoomSettings settings {
        MAX_PLAYERS, // roomSize par défaut
        NB_BOTS,     // nbBot par défaut
        300          // gameDuration (ex: secondes)
    };

};
