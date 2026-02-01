#pragma once

#include <gf/Scene.h>
#include <gf/Font.h>

#include "../common/Protocol.h"
#include "LobbyEntity.h"

class ClientGame;

class LobbyScene : public gf::Scene {
public:
    explicit LobbyScene(ClientGame& game);

    void setInitialState(
        const std::vector<PlayerData>& players,
        const RoomSettings& settings
    );

private:
    void doProcessEvent(gf::Event& event) override;
    void doUpdate(gf::Time time) override;

private:
    ClientGame& m_game;

    gf::Font m_font;
    LobbyEntity m_entity;

    std::vector<PlayerData> m_players;
    RoomSettings m_roomSettings{};
    bool m_amReady = false;
};
