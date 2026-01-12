#include "Lobby.h"

Lobby::Lobby(ServerNetwork& network)
: network(network)
, nextRoomId(1)
{
    gf::Log::info("[Lobby] Initialisation\n");

    defaultRoom = createRoom();
    gf::Log::info("[Lobby] Room par défaut = %u\n", defaultRoom);
}

void Lobby::onPlayerConnected(uint32_t playerId) {
    gf::Log::info("[Lobby] Joueur %u connecté\n", playerId);
    playerRoom[playerId] = 0; // pas encore de room
}



void Lobby::onPlayerDisconnected(uint32_t playerId) {
    gf::Log::info("[Lobby] Joueur %u déconnecté\n", playerId);

    auto it = playerRoom.find(playerId);
    if (it == playerRoom.end())
        return;

    RoomId roomId = it->second;
    auto roomIt = rooms.find(roomId);

    if (roomIt != rooms.end()) {
        roomIt->second->removePlayer(playerId);
        gf::Log::info("[Lobby] Joueur %u retiré de la room %u\n", playerId, roomId);
    }

    playerRoom.erase(it);
}

void Lobby::handlePacket(PacketContext& ctx) {
    //gf::Log::info("[Lobby] Paquet reçu du joueur %u (type=%llu)\n",ctx.senderId, static_cast<unsigned long long>(ctx.packet.getType()));

    if (ctx.packet.getType() == ClientJoinRoom::type) {
        handleClientJoinRoom(ctx);
        return;
    }

    auto it = playerRoom.find(ctx.senderId);
    if (it == playerRoom.end() || it->second == 0) {
        gf::Log::warning("[Lobby] Joueur %u sans room, paquet ignoré\n", ctx.senderId);
        return;
    }

    RoomId roomId = it->second;
    auto roomIt = rooms.find(roomId);
    if (roomIt == rooms.end()) {
        gf::Log::error("[Lobby] Room %u introuvable\n", roomId);
        return;
    }

    //gf::Log::info("[Lobby] Transmission du paquet du joueur %u vers room %u\n",ctx.senderId,roomId);

    roomIt->second->handlePacket(ctx);
}


RoomId Lobby::createRoom() {
    RoomId id = nextRoomId++;

    gf::Log::info("[Lobby] Création room %u\n", id);

    rooms.emplace(
        id,
        std::make_unique<Room>(id, network)
    );

    return id;
}

void Lobby::destroyRoom(RoomId id) {
    gf::Log::info("[Lobby] Destruction room %u\n", id);
    rooms.erase(id);
}

Room& Lobby::getRoom(uint32_t id) {
    return *rooms.at(id);
}

void Lobby::handleClientJoinRoom(PacketContext& ctx) {
    auto data = ctx.packet.as<ClientJoinRoom>();

    RoomId roomId = data.room;

    if (roomId == 0) {
        roomId = defaultRoom;
    }

    if (rooms.find(roomId) == rooms.end()) {
        gf::Log::info("[Lobby] Room %u inexistante, création\n", roomId);
        roomId = createRoom();
    }

    auto& room = rooms.at(roomId);
    if (room->isFull()) {
        gf::Log::info(
            "[Lobby] Room %u pleine (%zu / %u), création d'une nouvelle room\n",
            roomId,
            room->players.size(),
            room->getMaxPlayers()
        );
        roomId = createRoom();
    }


    playerRoom[ctx.senderId] = roomId;
    rooms[roomId]->addPlayer(ctx.senderId);

    gf::Log::info("[Lobby] Joueur %u ajouté à la room %u via ClientJoinRoom\n",
                  ctx.senderId, roomId);
}
