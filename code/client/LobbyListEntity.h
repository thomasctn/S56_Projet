#pragma once

#include <gf/Event.h>
#include <gf/WidgetContainer.h>
#include <gf/Widgets.h>
#include <gf/Text.h>
#include <gf/Font.h>
#include <vector>
#include <memory>

#include "Renderer.h"
#include "../common/Types.h"

enum class LobbyListAction {
    None,
    CreateRoom,
    JoinRoom
};

class LobbyListEntity {
public:
    explicit LobbyListEntity(Renderer& renderer);

    LobbyListAction processEvent(const gf::Event& event);

    // rendering
    void render();

    //maj des romms en fonction de ce que le server donne
    void setRooms(const std::vector<RoomData>& rooms);


    LobbyListAction getAndResetLastAction();
    unsigned int getLastRoomId() const;

private:
    Renderer& m_renderer;
    gf::Font m_font;
    gf::WidgetContainer m_container;

    gf::TextButtonWidget m_createWidget;

    //tt les boutons join
    std::vector<std::unique_ptr<gf::TextButtonWidget>> m_joinWidgets;

    std::vector<RoomData> m_rooms;

    LobbyListAction m_lastAction;
    unsigned int m_lastRoomId;
};
