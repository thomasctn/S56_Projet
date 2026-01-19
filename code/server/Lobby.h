#pragma once

#include <unordered_map>
#include <memory>
#include <cstdint>
#include <gf/Log.h>
#include "PacketContext.h"
#include "ServerNetwork.h"
#include "Room.h"
#include "../common/Constants.h"
#include "../common/Protocol.h"



class ServerNetwork;
class Room;

using RoomId = uint32_t;

class Lobby {
public:
    Lobby(ServerNetwork& network);

    void onPlayerConnected(uint32_t playerId);
    void onPlayerDisconnected(uint32_t playerId);
    void handlePacket(PacketContext& ctx);
    Room& getRoom(uint32_t id);
    void handleClientJoinRoom(PacketContext& ctx);
    void broadcastRoomsList();
    void sendRoomsList(uint32_t playerId);
    ServerListRooms getRoomsList() const;


private:
    RoomId createRoom(const std::string& hostName);
    void destroyRoom(RoomId id);

private:
    ServerNetwork& network;
    std::unordered_map<RoomId, std::unique_ptr<Room>> rooms;
    std::unordered_map<uint32_t, RoomId> playerRoom;
    RoomId nextRoomId;
    RoomId defaultRoom;
};
