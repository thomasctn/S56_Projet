#include "LobbyListEntity.h"

#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Log.h>
#include <gf/Anchor.h>
#include <gf/Coordinates.h>

LobbyListEntity::LobbyListEntity(Renderer& renderer)
: m_renderer(renderer)
, m_font("../common/fonts/arial.ttf")
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

    while (m_joinWidgets.size() < m_rooms.size()) { // creer autant de bouton que de rooms
        auto ptr = std::make_unique<gf::TextButtonWidget>("Joindre", m_font);
        ptr->setCharacterSize(18);
        ptr->setAnchor(gf::Anchor::Center);

        ptr->setCallback([this]() { //pr l'instant rien, on le met proprement avec les rooms apres
        });

        m_container.addWidget(*ptr);
        m_joinWidgets.push_back(std::move(ptr));
    }

    for (size_t i = 0; i < m_rooms.size(); ++i) {
        RoomData const &rd = m_rooms[i];
        auto &w = m_joinWidgets[i];

        w->setString("Joindre");

        unsigned int roomId = rd.roomID;
        w->setCallback([this, roomId]() {
            m_lastRoomId = roomId;
            m_lastAction = LobbyListAction::JoinRoom;
        });
    }
}

LobbyListAction LobbyListEntity::processEvent(const gf::Event& event) {
    gf::RenderWindow& win = m_renderer.getRenderWindow();

    switch (event.type) {
        case gf::EventType::MouseMoved: {
            gf::Vector2i pixelPos(int(event.mouseCursor.coords.x), int(event.mouseCursor.coords.y));
            gf::Vector2f worldPos = win.mapPixelToCoords(pixelPos);
            m_container.pointTo(worldPos);
            return LobbyListAction::None;
        }

        case gf::EventType::MouseButtonPressed: {
            if (event.mouseButton.button != gf::MouseButton::Left) return LobbyListAction::None;
            gf::Vector2i pixelPos(int(event.mouseButton.coords.x), int(event.mouseButton.coords.y));
            gf::Vector2f worldPos = win.mapPixelToCoords(pixelPos);
            m_container.pointTo(worldPos);
            m_container.triggerAction();

            LobbyListAction a = m_lastAction;
            m_lastAction = LobbyListAction::None;
            return a;
        }

        default:
            return LobbyListAction::None;
    }
}

LobbyListAction LobbyListEntity::getAndResetLastAction() {
    LobbyListAction a = m_lastAction;
    m_lastAction = LobbyListAction::None;
    return a;
}

unsigned int LobbyListEntity::getLastRoomId() const {
    return m_lastRoomId;
}

void LobbyListEntity::render() {
    m_renderer.clearWindow();
    gf::RenderWindow& target = m_renderer.getRenderWindow();

    gf::View view = target.getView();
    gf::Vector2f center = view.getCenter();
    gf::Vector2f size = view.getSize();

    float left = center.x - size.x * 0.5f;
    float top  = center.y - size.y * 0.5f;
    float width = size.x;
    float height = size.y;

    float margin = 16.f;
    float y = top + margin;

    float bw = width * 0.3f; //pr le bouton créer
    float bh = height * 0.08f;
    float bx = left + (width - bw) * 0.5f;
    float by = y;

    gf::RectangleShape createBg({ bw, bh }); //background deco
    createBg.setPosition({ bx, by });
    createBg.setColor(gf::Color::fromRgb(40, 120, 200));
    target.draw(createBg);

    m_createWidget.setCharacterSize(20);
    m_createWidget.setAnchor(gf::Anchor::Center);
    m_createWidget.setPosition({ bx + bw * 0.5f, by + bh * 0.5f });
    target.draw(m_createWidget);

    y += bh + margin;

    //header pr le titre
    gf::Text header;
    header.setFont(m_font);
    header.setCharacterSize(18);
    header.setColor(gf::Color::White);
    header.setString("Rooms disponibles :");
    header.setPosition({ left + margin, y });
    target.draw(header);

    y += 28.f;

    float rowH = std::max(28.f, bh * 0.8f);
    float labelW = width * 0.6f;
    float btnW = width * 0.2f;

    for (size_t i = 0; i < m_rooms.size(); ++i) { //pour chaque room on dessine des rectangles
        const RoomData &rd = m_rooms[i];

        float rowX = left + margin;
        float rowY = y + float(i) * (rowH + 8.f);

        gf::RectangleShape rowBg({ width - margin * 2.f, rowH });
        rowBg.setPosition({ rowX, rowY });
        if (i % 2 == 0) 
            rowBg.setColor(gf::Color::fromRgb(30,30,30));
        else 
            rowBg.setColor(gf::Color::fromRgb(45,45,45));
        target.draw(rowBg);

        //nom du hosts, joueurs
        gf::Text roomText;
        roomText.setFont(m_font);
        roomText.setCharacterSize(16);
        roomText.setColor(gf::Color::White);

        std::string label = rd.hostName + "  (" + std::to_string(rd.nbPlayer) + " / " + std::to_string(rd.roomSize) + ")";
        roomText.setString(label);
        roomText.setPosition({ rowX + 8.f, rowY + rowH * 0.175f });
        target.draw(roomText);

        //pr le bouton join
        float joinX = left + width - margin - btnW;
        float joinY = rowY + (rowH - (rowH * 0.8f)) * 0.5f;
        gf::RectangleShape joinBg({ btnW, rowH * 0.8f });
        joinBg.setPosition({ joinX, joinY });
        joinBg.setColor(gf::Color::fromRgb(70, 140, 70));
        target.draw(joinBg);

        //veirfie qu'on a bien fait un widget
        if (i < m_joinWidgets.size()) {
            auto &w = *m_joinWidgets[i];
            w.setCharacterSize(16);
            w.setAnchor(gf::Anchor::Center);
            w.setPosition({ joinX + btnW * 0.5f, joinY + (rowH * 0.8f) * 0.5f });
            target.draw(w);
        }
    }

    target.display();
}
