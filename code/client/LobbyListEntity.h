#pragma once

#include <gf/Entity.h>
#include <gf/Font.h>
#include <gf/WidgetContainer.h>
#include <gf/Widgets.h>

#include "../common/Protocol.h"

enum class LobbyListAction {
    None,
    CreateRoom,
    JoinRoom
};

class LobbyListEntity : public gf::Entity {
public:
    LobbyListEntity();

    void setRooms(const std::vector<RoomData>& rooms);

    void pointTo(gf::Vector2f position);
    void triggerAction();

    LobbyListAction getAndResetLastAction();
    unsigned int getLastRoomId() const;

    void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

private:
    gf::Font m_font;
    gf::WidgetContainer m_container;
    gf::TextButtonWidget m_createWidget;
    std::vector<std::unique_ptr<gf::TextButtonWidget>> m_joinWidgets;

    std::vector<RoomData> m_rooms;

    LobbyListAction m_lastAction;
    unsigned int m_lastRoomId;
};
