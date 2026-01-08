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

class Room {
public:
    Room(uint32_t id, ServerNetwork& network);

    void addPlayer(uint32_t playerId);
    void removePlayer(uint32_t playerId);

    void handlePacket(PacketContext& ctx);

    Game& getGameRef() { return *game; }

    std::unique_ptr<Game> game;
    uint32_t id;
    ServerNetwork& network;
    std::unordered_set<uint32_t> players;
    InputQueue inputQueue;

    bool gameStarted = false;
    Room* room = nullptr;
    void broadcastState();


private:
    void startGame();
    void handleClientMove(PacketContext& ctx);
    void setRoom(Room& r) { room = &r; }

};
