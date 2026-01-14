#pragma once
#include <gf/Event.h>
#include <gf/WidgetContainer.h>
#include <gf/Widgets.h>
#include <gf/Text.h>
#include <gf/Font.h>
#include "Renderer.h"
#include "../common/Types.h"

enum class LobbyAction {// actions possibles renvoyées par l'entity au main/scene

    None,
    RoomDec,
    RoomInc,
    BotDec,
    BotInc,
    DurDec,
    DurInc,
    ToggleReady,
    ChangeRole
};

class LobbyEntity {
public:
    explicit LobbyEntity(Renderer& renderer);

    LobbyAction processEvent(const gf::Event& event);

    void render(int connectedPlayers,int roomSize,bool amReady,int nbBots,int gameDur, PlayerRole myRole);

private:
    Renderer& m_renderer;
    gf::Font m_font;
    gf::WidgetContainer m_container;
    gf::TextWidget m_minusBtn;
    gf::TextWidget m_plusBtn;
    gf::TextWidget m_minusBotBtn;
    gf::TextWidget m_plusBotBtn;
    gf::TextWidget m_minusDurBtn;
    gf::TextWidget m_plusDurBtn;
    gf::TextWidget m_readyBtn;
    gf::TextWidget m_changeRoleBtn;

    LobbyAction m_lastAction;
};
