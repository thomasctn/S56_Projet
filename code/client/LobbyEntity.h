#pragma once

#include <gf/Entity.h>
#include <gf/WidgetContainer.h>
#include <gf/Widgets.h>
#include <gf/Text.h>
#include <gf/Font.h>
#include <vector>
#include <memory>

#include "../common/Protocol.h" 


enum class LobbyAction {
    None,

    Leave,

    RoomDec,
    RoomInc,

    BotDec,
    BotInc,

    DurDec,
    DurInc,

    ToggleReady,
    ChangeRole
};

class LobbyEntity : public gf::Entity {
public:
  LobbyEntity();

  void pointTo(gf::Vector2f coords);
  void triggerAction();
  void setPlayers(const std::vector<PlayerData>& players);
  void setRoomSettings(const RoomSettings& settings);
  void setClientId(uint32_t id);

  LobbyAction getAndResetLastAction();

  void render(gf::RenderTarget& target, const gf::RenderStates& states) override;


    
private:
  void renderPlayerRow(gf::RenderTarget& target, const gf::RenderStates& states, gf::Vector2f position, const PlayerData& data);
  void renderPlayerList(gf::RenderTarget& target, const gf::RenderStates& states, gf::Vector2f position);
  void renderRoleSelection(gf::RenderTarget& target, const gf::RenderStates& states, gf::Vector2f position);
  void renderSettings(gf::RenderTarget& target, const gf::RenderStates& states, gf::Vector2f position);
  void defaultButtonColor(gf::TextButtonWidget& w);



private:
  gf::Font m_font;

  gf::WidgetContainer m_container;
  gf::TextButtonWidget m_leaveBtn;
  gf::TextButtonWidget m_minusBtn;
  gf::TextButtonWidget m_plusBtn;
  gf::TextButtonWidget m_minusBotBtn;
  gf::TextButtonWidget m_plusBotBtn;
  gf::TextButtonWidget m_minusDurBtn;
  gf::TextButtonWidget m_plusDurBtn;
  gf::TextButtonWidget m_readyBtn;
  gf::TextButtonWidget m_changeRoleBtn;

  LobbyAction m_lastAction = LobbyAction::None;

  //état affiché
  std::vector<PlayerData> m_players;
  RoomSettings m_roomSettings{};
  uint32_t m_clientId = 0;
};
