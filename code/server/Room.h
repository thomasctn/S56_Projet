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
    bool allPlayersReady() const;
    bool isFull() const {return players.size() >= settings.roomSize;}
    void endGame(GameEndReason reason);
    void notifyGameEnded(GameEndReason reason);
    void cleanupGame();
    void onGameStopped();
    void notifyGameEndedAsync(GameEndReason reason);


    void handlePacket(PacketContext& ctx);
    void handleClientChange(PacketContext& ctx);
    void handleClientReady(PacketContext& ctx);
    void handleClientMove(PacketContext& ctx);
    void handleClientChangeRoomSettings(PacketContext& ctx);


    void broadcastState();
    void broadcastRoomPlayers();
    void broadcastRoomSettings();
    void broadcastPreGame(unsigned int timeLeft);

    Game& getGame() { return *game; }
    PlayerData getPlayerData(uint32_t playerId) const;
    const RoomSettings& getSettings() const { return settings; }
    unsigned int getMaxPlayers() const {return settings.roomSize;}
    unsigned int getGameDuration() const { return settings.gameDuration; }
    BotManager* getBotManager() { return botManager.get(); }


    void setSettings(const RoomSettings& newSettings);
    void setHostName(const std::string& name) { hostName = name; }
    const std::string& getHostName() const { return hostName; }
    unsigned int getPlayerCount() const { return static_cast<unsigned int>(players.size()); }

    std::unique_ptr<Game> game;
    uint32_t id;
    ServerNetwork& network;
    std::unordered_set<uint32_t> players;
    InputQueue inputQueue;
    std::unordered_map<uint32_t, PlayerData> preGamePlayers;
    Room* room = nullptr;



private:
    void startGame();

    void setRoom(Room& r) { room = &r; }
    uint32_t generateBotId();
    void resetPlayersState();

    std::string hostName;
    std::unique_ptr<BotManager> botManager;
    RoomSettings settings {
        MAX_PLAYERS, // roomSize par défaut
        NB_BOTS,     // nbBot par défaut
        MIN_DURATION // gameDuration (ex: secondes)
    };

};
