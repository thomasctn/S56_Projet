#pragma once
#include <gf/Event.h>
#include "LobbyEntity.h"
#include "Renderer.h"

class LobbyScene {
public:
    explicit LobbyScene(Renderer& renderer);

    LobbyAction processEvent(const gf::Event& event);

    void render(std::vector<PlayerData> players,RoomSettings settings, int clientID);

private:
    LobbyEntity m_entity;
};
