#pragma once
#include <gf/Event.h>
#include "LobbyEntity.h"
#include "Renderer.h"

class LobbyScene {
public:
    explicit LobbyScene(Renderer& renderer);

    LobbyAction processEvent(const gf::Event& event);

    void render(int connectedPlayers,int roomSize,bool amReady,int nbBots,int gameDur, PlayerRole myRole);

private:
    LobbyEntity m_entity;
};
