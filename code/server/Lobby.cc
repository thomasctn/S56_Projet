#include "Lobby.h"

Lobby::Lobby(ServerNetwork& network)
: network(network)
, nextRoomId(1)
{
    gf::Log::info("[Lobby] Initialisation\n");

    // Création de la room par défaut
    defaultRoom = createRoom("Default");
    gf::Log::info("[Lobby] Room par défaut = %u\n", defaultRoom);
}

void Lobby::onPlayerConnected(uint32_t playerId) {
    gf::Log::info("[Lobby] Joueur %u connecté\n", playerId);

    playerRoom[playerId] = 0;

    sendRoomsList(playerId);
}

void Lobby::onPlayerDisconnected(uint32_t playerId) {
    gf::Log::info("[Lobby] Joueur %u déconnecté\n", playerId);

    auto it = playerRoom.find(playerId);
    if (it != playerRoom.end()) {
        RoomId roomId = it->second;
        auto roomIt = rooms.find(roomId);

        if (roomIt != rooms.end()) {
            roomIt->second->removePlayer(playerId);
            gf::Log::info("[Lobby] Joueur %u retiré de la room %u\n", playerId, roomId);
        }

        playerRoom.erase(it);
        broadcastRoomsList();
    }
}

void Lobby::handlePacket(PacketContext& ctx) {
    switch (ctx.packet.getType()) {
        case ClientJoinRoom::type:
            handleClientJoinRoom(ctx);
            break;

        default:
            // Transmettre le paquet à la room si le joueur y est
            if (playerRoom.find(ctx.senderId) != playerRoom.end() && playerRoom[ctx.senderId] != 0) {
                RoomId roomId = playerRoom[ctx.senderId];
                auto roomIt = rooms.find(roomId);
                if (roomIt != rooms.end()) {
                    roomIt->second->handlePacket(ctx);
                } else {
                    gf::Log::error("[Lobby] Room %u introuvable pour joueur %u\n", roomId, ctx.senderId);
                }
            } else {
                gf::Log::warning("[Lobby] Joueur %u sans room, paquet ignoré\n", ctx.senderId);
            }
            break;
    }
}

RoomId Lobby::createRoom(const std::string& hostName) {
    RoomId id = nextRoomId++;
    gf::Log::info("[Lobby] Création room %u\n", id);

    auto room = std::make_unique<Room>(id, network);
    room->setHostName(hostName);
    rooms[id] = std::move(room);

    broadcastRoomsList();
    return id;
}

void Lobby::destroyRoom(RoomId id) {
    gf::Log::info("[Lobby] Destruction room %u\n", id);
    rooms.erase(id);
    broadcastRoomsList();
}

Room& Lobby::getRoom(uint32_t id) {
    return *rooms.at(id);
}

void Lobby::handleClientJoinRoom(PacketContext& ctx) {
    auto data = ctx.packet.as<ClientJoinRoom>();
    RoomId roomId = data.room;

    // Si le client ne spécifie pas de room, utiliser default
    if (roomId == 0)
        roomId = defaultRoom;

    // Créer une room si elle n’existe pas
    if (rooms.find(roomId) == rooms.end()) {
        gf::Log::info("[Lobby] Room %u inexistante, création\n", roomId);
        roomId = createRoom("Player_" + std::to_string(ctx.senderId));
    }

    auto& room = rooms.at(roomId);
    if (room->isFull()) {
        gf::Log::info("[Lobby] Room %u pleine (%u/%u), création d'une nouvelle room\n",
                      roomId, room->getPlayerCount(), room->getMaxPlayers());
        roomId = createRoom("Player_" + std::to_string(ctx.senderId));
    }

    // Ajouter le joueur à la room
    playerRoom[ctx.senderId] = roomId;
    rooms.at(roomId)->addPlayer(ctx.senderId);

    gf::Packet joinAck;
    joinAck.is(ServerJoinRoom{});
    network.send(ctx.senderId, joinAck);

    broadcastRoomsList();

    gf::Log::info("[Lobby] Joueur %u ajouté à la room %u\n", ctx.senderId, roomId);
}

ServerListRooms Lobby::getRoomsList() const {
    ServerListRooms msg;
    for (auto& [id, roomPtr] : rooms) {
        RoomData data;
        data.roomID = id;
        data.roomSize = roomPtr->getMaxPlayers();
        data.nbPlayer = roomPtr->getPlayerCount();
        data.hostName = roomPtr->getHostName();
        msg.rooms.push_back(data);
    }
    return msg;
}

void Lobby::sendRoomsList(uint32_t playerId) {
    ServerListRooms msg = getRoomsList();
    gf::Packet packet;
    packet.is(msg);
    network.send(playerId, packet);
}

void Lobby::broadcastRoomsList() {
    ServerListRooms msg = getRoomsList();
    gf::Packet packet;
    packet.is(msg);

    for (auto& [pid, _] : playerRoom)
        network.send(pid, packet);
}
