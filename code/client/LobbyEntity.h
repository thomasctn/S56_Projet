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

    void render(std::vector<PlayerData> players,RoomSettings settings, int clientID);
    void renderPlayerRow(gf::Vector2f position,PlayerData data, bool isHost);
    void renderPLayerList(gf::Vector2f position, std::vector<PlayerData>);
    void renderRoleSelection(gf::Vector2f position, PlayerRole currentRole);
    void renderSettings(gf::Vector2f position, RoomSettings settings);

private:
    Renderer& m_renderer;
    gf::Font m_font;
    gf::WidgetContainer m_container;
    gf::TextButtonWidget m_minusBtn;
    gf::TextButtonWidget m_plusBtn;
    gf::TextButtonWidget m_minusBotBtn;
    gf::TextButtonWidget m_plusBotBtn;
    gf::TextButtonWidget m_minusDurBtn;
    gf::TextButtonWidget m_plusDurBtn;
    gf::TextButtonWidget m_readyBtn;
    gf::TextButtonWidget m_changeRoleBtn;

    LobbyAction m_lastAction;
};
