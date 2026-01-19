#pragma once

#include "LobbyListEntity.h"
#include "Renderer.h"

class LobbyListScene {
public:
    explicit LobbyListScene(Renderer& renderer);

    LobbyListAction processEvent(const gf::Event& event);

    void render();

    void setRooms(const std::vector<RoomData>& rooms);

    LobbyListAction getAndResetLastAction();
    unsigned int getLastRoomId() const;

private:
    LobbyListEntity m_entity;
};
