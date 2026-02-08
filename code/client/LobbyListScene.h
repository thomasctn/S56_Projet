#pragma once

#include <gf/Scene.h>
#include <gf/Font.h>
#include <gf/Packet.h>

#include "LobbyListEntity.h"

class ClientGame;

class LobbyListScene : public gf::Scene {
public:
    explicit LobbyListScene(ClientGame& game);
    LobbyListEntity& getEntity() { return m_entity; }
    void resizeYourself() ;



private:
    void doProcessEvent(gf::Event& event) override;
    void doUpdate(gf::Time time) override;

private:
    ClientGame& m_game;
    LobbyListEntity m_entity;
};
