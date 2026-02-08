#include "LobbyListEntity.h"

#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Anchor.h>
#include <gf/Text.h>
#include <gf/Shapes.h>
#include <gf/Log.h>

LobbyListEntity::LobbyListEntity()
: m_font("../common/fonts/arial.ttf")
, m_createWidget("Créer une room", m_font)
, m_lastAction(LobbyListAction::None)
, m_lastRoomId(0)
{
    m_createWidget.setCallback([this]() {
        m_lastAction = LobbyListAction::CreateRoom;
    });

    m_container.addWidget(m_createWidget);

    m_createWidget.setCharacterSize(24);
    m_createWidget.setAnchor(gf::Anchor::Center);
}

void LobbyListEntity::setRooms(const std::vector<RoomData>& rooms) {
    m_rooms = rooms;

    while (m_joinWidgets.size() < m_rooms.size()) {
        auto ptr = std::make_unique<gf::TextButtonWidget>("Joindre", m_font);
        ptr->setCharacterSize(18);
        ptr->setAnchor(gf::Anchor::Center);

        m_container.addWidget(*ptr);
        m_joinWidgets.push_back(std::move(ptr));
    }

    for (size_t i = 0; i < m_rooms.size(); ++i) {
        unsigned int roomId = m_rooms[i].roomID;

        m_joinWidgets[i]->setCallback([this, roomId]() {
            m_lastRoomId = roomId;
            m_lastAction = LobbyListAction::JoinRoom;
        });
    }
}

void LobbyListEntity::pointTo(gf::Vector2f position) {
    m_container.pointTo(position);
}

void LobbyListEntity::triggerAction() {
    m_container.triggerAction();
}

LobbyListAction LobbyListEntity::getAndResetLastAction() {
    LobbyListAction a = m_lastAction;
    m_lastAction = LobbyListAction::None;
    return a;
}

unsigned int LobbyListEntity::getLastRoomId() const {
    return m_lastRoomId;
}

void LobbyListEntity::render(gf::RenderTarget& target, const gf::RenderStates& states) {
    const float LOGICAL_W = 1280.f;
    const float LOGICAL_H = 720.f;

    float margin = 16.f;
    float y = margin;

    float bw = LOGICAL_W * 0.3f;
    float bh = LOGICAL_H * 0.08f;
    float bx = (LOGICAL_W - bw) * 0.5f;
    float by = y;

    m_createWidget.setCharacterSize(26);
    m_createWidget.setAnchor(gf::Anchor::Center);
    m_createWidget.setPosition({ bx + bw * 0.5f, by + bh * 0.5f });

    m_createWidget.setDefaultTextColor(gf::Color::White);
    m_createWidget.setSelectedTextColor(gf::Color::Black);
    m_createWidget.setDefaultBackgroundColor(gf::Color::Black);
    m_createWidget.setSelectedBackgroundColor(gf::Color::White);
    m_createWidget.setBackgroundOutlineThickness(26 * .05f);
    m_createWidget.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_createWidget.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_createWidget.setPadding(26 * .4f);
    target.draw(m_createWidget, states);

    y += bh + margin;

    gf::Text header;
    header.setFont(m_font);
    header.setCharacterSize(18);
    header.setColor(gf::Color::White);
    header.setString("Rooms disponibles :");
    header.setPosition({ margin, y });
    target.draw(header, states);

    y += 28.f;

    float rowH = std::max(28.f, bh * 0.8f);
    float btnW = LOGICAL_W * 0.2f;

    for (size_t i = 0; i < m_rooms.size(); ++i) {
        const RoomData& rd = m_rooms[i];

        float rowX = margin;
        float rowY = y + float(i) * (rowH + 8.f);

        gf::RectangleShape rowBg({ LOGICAL_W - margin * 2.f, rowH });
        rowBg.setPosition({ rowX, rowY });
        rowBg.setColor((i % 2 == 0) ? gf::Color::fromRgb(30,30,30) : gf::Color::fromRgb(45,45,45));
        target.draw(rowBg, states);

        gf::Text roomText;
        roomText.setFont(m_font);
        roomText.setCharacterSize(16);
        roomText.setColor(gf::Color::Black);
        roomText.setString(rd.hostName + " (" + std::to_string(rd.nbPlayer) + " / " + std::to_string(rd.roomSize) + ")");
        roomText.setPosition({ rowX + 8.f, rowY + rowH * 0.175f });
        target.draw(roomText, states);

        if (i < m_joinWidgets.size()) {
            auto& w = *m_joinWidgets[i];

            float joinX = LOGICAL_W - margin - btnW;
            float joinY = rowY + (rowH - rowH * 0.8f) * 0.5f;

            w.setCharacterSize(16);
            w.setAnchor(gf::Anchor::Center);
            w.setPosition({ joinX + btnW * 0.5f, joinY + (rowH * 0.8f) * 0.5f });

            w.setDefaultTextColor(gf::Color::White);
            w.setSelectedTextColor(gf::Color::Black);
            w.setDefaultBackgroundColor(gf::Color::Black);
            w.setSelectedBackgroundColor(gf::Color::White);
            w.setBackgroundOutlineThickness(16 * .05f);
            w.setDefaultBackgroundOutlineColor(gf::Color::White);
            w.setSelectedBackgroundOutlineColor(gf::Color::White);
            w.setPadding(16 * .4f);

            target.draw(w, states);
        }
    }
}
