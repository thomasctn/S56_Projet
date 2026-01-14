#include "LobbyEntity.h"
#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Log.h>
#include <gf/Anchor.h>

LobbyEntity::LobbyEntity(Renderer& renderer)
: m_renderer(renderer)
, m_font("../common/fonts/arial.ttf")
, m_minusBtn("-", m_font)
, m_plusBtn("+", m_font)
, m_minusBotBtn("-", m_font)
, m_plusBotBtn("+", m_font)
, m_minusDurBtn("-", m_font)
, m_plusDurBtn("+", m_font)
, m_readyBtn("PRÊT", m_font)
, m_changeRoleBtn("Changer", m_font)
, m_lastAction(LobbyAction::None)
{
    m_minusBtn.setCallback([this](){ m_lastAction = LobbyAction::RoomDec; });
    m_plusBtn.setCallback([this](){ m_lastAction = LobbyAction::RoomInc; });
    m_minusBotBtn.setCallback([this](){ m_lastAction = LobbyAction::BotDec; });
    m_plusBotBtn.setCallback([this](){ m_lastAction = LobbyAction::BotInc; });
    m_minusDurBtn.setCallback([this](){ m_lastAction = LobbyAction::DurDec; });
    m_plusDurBtn.setCallback([this](){ m_lastAction = LobbyAction::DurInc; });
    m_readyBtn.setCallback([this](){ m_lastAction = LobbyAction::ToggleReady; });
    m_changeRoleBtn.setCallback([this](){ m_lastAction = LobbyAction::ChangeRole; });

    m_container.addWidget(m_minusBtn); //met widgets dans le container
    m_container.addWidget(m_plusBtn);
    m_container.addWidget(m_minusBotBtn);
    m_container.addWidget(m_plusBotBtn);
    m_container.addWidget(m_minusDurBtn);
    m_container.addWidget(m_plusDurBtn);
    m_container.addWidget(m_readyBtn);
    m_container.addWidget(m_changeRoleBtn);

    unsigned char defaultCharSize = 20;
    for (auto &w : { &m_minusBtn, &m_plusBtn, &m_minusBotBtn, &m_plusBotBtn, &m_minusDurBtn, &m_plusDurBtn }) {
        w->setCharacterSize(24);
        w->setAnchor(gf::Anchor::Center);
    }
    m_readyBtn.setCharacterSize(20);
    m_readyBtn.setAnchor(gf::Anchor::Center);
    m_changeRoleBtn.setCharacterSize(18);
    m_changeRoleBtn.setAnchor(gf::Anchor::Center);
}
LobbyAction LobbyEntity::processEvent(const gf::Event& event){
    gf::RenderWindow& win = m_renderer.getRenderWindow();

    switch (event.type) {
        case gf::EventType::MouseMoved: {
            gf::Vector2i pixelPos(int(event.mouseCursor.coords.x), int(event.mouseCursor.coords.y));
            gf::Vector2f worldPos = win.mapPixelToCoords(pixelPos);
            m_container.pointTo(worldPos);
            return LobbyAction::None;
        }

        case gf::EventType::MouseButtonPressed: {
            if (event.mouseButton.button != gf::MouseButton::Left) return LobbyAction::None;
            gf::Vector2i pixelPos(int(event.mouseButton.coords.x), int(event.mouseButton.coords.y));
            gf::Vector2f worldPos = win.mapPixelToCoords(pixelPos);
            m_container.pointTo(worldPos);
            m_container.triggerAction();

            LobbyAction a = m_lastAction;
            m_lastAction = LobbyAction::None;
            return a;
        }

        default:
            return LobbyAction::None;
    }
}


void LobbyEntity::render(int connectedPlayers, int roomSize, bool amReady, int nbBots, int gameDur, PlayerRole myRole) {
    gf::RenderWindow& target = m_renderer.getRenderWindow();
    m_renderer.clearWindow();

    float worldSize = m_renderer.getWorldSize();
    float margin = 20.f;

    // positions boutons
    gf::Vector2f btnSize{40.f, 40.f};
    gf::Vector2f minusBtnPos{margin + 140.f, 100.f};
    gf::Vector2f plusBtnPos{minusBtnPos.x + btnSize.x + 60.f, 100.f};

    gf::Vector2f minusBotBtnPos{margin + 140.f, 150.f};
    gf::Vector2f plusBotBtnPos{minusBotBtnPos.x + btnSize.x + 60.f, 150.f};

    gf::Vector2f minusDurBtnPos{margin + 140.f, 200.f};
    gf::Vector2f plusDurBtnPos{minusDurBtnPos.x + btnSize.x + 60.f, 200.f};

    gf::Vector2f changeRolePos{margin + 140.f, 250.f};

    gf::Vector2f readyBtnSize{worldSize * 0.18f, worldSize * 0.08f};
    gf::Vector2f readyBtnPos{margin, 420.f};

    
    static gf::Font font("../common/fonts/arial.ttf");

    gf::Text title;
    title.setFont(font);
    title.setCharacterSize(28);
    title.setColor(gf::Color::White);
    title.setString("En attente de joueurs...");
    title.setPosition({margin, 20.f});
    target.draw(title);

    gf::Text countText;
    countText.setFont(font);
    countText.setCharacterSize(20);
    countText.setColor(gf::Color::White);
    countText.setString(std::to_string(connectedPlayers) + " / " + std::to_string(roomSize));
    countText.setPosition({margin, 60.f});
    target.draw(countText);

    gf::Text roomLabel;
    roomLabel.setFont(font);
    roomLabel.setCharacterSize(20);
    roomLabel.setColor(gf::Color::White);
    roomLabel.setString("Joueurs max :");
    roomLabel.setPosition({margin,100.f});
    target.draw(roomLabel);

    // rectangle de fond réutilisables comme avant
    gf::RectangleShape back(btnSize);
    back.setColor(gf::Color::fromRgb(50, 70, 200)); // couleur de fond des petits boutons pas ça marche moeyn

    
    back.setPosition(minusBtnPos);
    target.draw(back);

    m_minusBtn.setCharacterSize(24);
    m_minusBtn.setAnchor(gf::Anchor::Center);
    m_minusBtn.setPosition({ minusBtnPos.x + btnSize.x * 0.5f, minusBtnPos.y + btnSize.y * 0.5f });
    target.draw(m_minusBtn);

    back.setPosition(plusBtnPos);
    target.draw(back);

    m_plusBtn.setCharacterSize(24);
    m_plusBtn.setAnchor(gf::Anchor::Center);
    m_plusBtn.setPosition({ plusBtnPos.x + btnSize.x * 0.5f, plusBtnPos.y + btnSize.y * 0.5f });
    target.draw(m_plusBtn);

    gf::Text valueText;
    valueText.setFont(font);
    valueText.setCharacterSize(20);
    valueText.setColor(gf::Color::White);
    valueText.setString(std::to_string(roomSize));
    valueText.setPosition({ minusBtnPos.x + btnSize.x + 10.f, minusBtnPos.y + 8.f });
    target.draw(valueText);

    //Bots +/- 
    gf::Text botLabel;
    botLabel.setFont(font);
    botLabel.setCharacterSize(20);
    botLabel.setColor(gf::Color::White);
    botLabel.setString("Nb de bots :");
    botLabel.setPosition({margin,150.f});
    target.draw(botLabel);

    back.setPosition(minusBotBtnPos);
    target.draw(back);
    m_minusBotBtn.setCharacterSize(24);
    m_minusBotBtn.setAnchor(gf::Anchor::Center);
    m_minusBotBtn.setPosition({ minusBotBtnPos.x + btnSize.x * 0.5f, minusBotBtnPos.y + btnSize.y * 0.5f });
    target.draw(m_minusBotBtn);

    back.setPosition(plusBotBtnPos);
    target.draw(back);
    m_plusBotBtn.setCharacterSize(24);
    m_plusBotBtn.setAnchor(gf::Anchor::Center);
    m_plusBotBtn.setPosition({ plusBotBtnPos.x + btnSize.x * 0.5f, plusBotBtnPos.y + btnSize.y * 0.5f });
    target.draw(m_plusBotBtn);

    valueText.setString(std::to_string(nbBots));
    valueText.setPosition({ minusBotBtnPos.x + btnSize.x + 10.f, minusBotBtnPos.y + 8.f });
    target.draw(valueText);

    //Duration +/- 
    gf::Text durationLabel;
    durationLabel.setFont(font);
    durationLabel.setCharacterSize(20);
    durationLabel.setColor(gf::Color::White);
    durationLabel.setString("Temps de jeu\n(secondes) :");
    durationLabel.setPosition({margin,200.f});
    target.draw(durationLabel);

    back.setPosition(minusDurBtnPos);
    target.draw(back);
    m_minusDurBtn.setCharacterSize(24);
    m_minusDurBtn.setAnchor(gf::Anchor::Center);
    m_minusDurBtn.setPosition({ minusDurBtnPos.x + btnSize.x * 0.5f, minusDurBtnPos.y + btnSize.y * 0.5f });
    target.draw(m_minusDurBtn);

    back.setPosition(plusDurBtnPos);
    target.draw(back);
    m_plusDurBtn.setCharacterSize(24);
    m_plusDurBtn.setAnchor(gf::Anchor::Center);
    m_plusDurBtn.setPosition({ plusDurBtnPos.x + btnSize.x * 0.5f, plusDurBtnPos.y + btnSize.y * 0.5f });
    target.draw(m_plusDurBtn);

    valueText.setString(std::to_string(gameDur));
    valueText.setPosition({ minusDurBtnPos.x + btnSize.x + 10.f, minusDurBtnPos.y + 8.f });
    target.draw(valueText);

    //Role text + CR button
    gf::Text roleLabel;
    roleLabel.setFont(font);
    roleLabel.setCharacterSize(20);
    roleLabel.setColor(gf::Color::White);
    if (myRole == PlayerRole::PacMan) {
        roleLabel.setString("Vous etes \nactuellement :\nPACMAN");
    } else {
        roleLabel.setString("Vous etes \nactuellement :\nUN FANTOME");
    }
    roleLabel.setPosition({margin, 250.f});
    target.draw(roleLabel);

    // CR rectangle 
    gf::RectangleShape crRect({ readyBtnSize.x * 0.5f, readyBtnSize.y });
    crRect.setPosition(changeRolePos);
    crRect.setColor(gf::Color::fromRgb(204, 77, 153));
    target.draw(crRect);

    m_changeRoleBtn.setCharacterSize(int(readyBtnSize.y * 0.35f));
    m_changeRoleBtn.setAnchor(gf::Anchor::Center);
    //centre du rect
    m_changeRoleBtn.setPosition({ changeRolePos.x + (readyBtnSize.x * 0.5f) * 0.5f, changeRolePos.y + readyBtnSize.y * 0.5f });
    if (myRole == PlayerRole::PacMan) {
        m_changeRoleBtn.setString("Devenir un fantome ?");
    } else {
        m_changeRoleBtn.setString("Devenir Pacman ?");
    }
    target.draw(m_changeRoleBtn);

    //Ready
    gf::RectangleShape readyRect(readyBtnSize);
    readyRect.setPosition(readyBtnPos);
    if (amReady) {
        readyRect.setColor(gf::Color::fromRgb(179, 51, 51));
    } else {
        readyRect.setColor(gf::Color::fromRgb(51, 179, 51));
    }
    target.draw(readyRect);

    m_readyBtn.setCharacterSize(int(readyBtnSize.y * 0.5f));
    m_readyBtn.setAnchor(gf::Anchor::Center);
    m_readyBtn.setString(amReady ? "PLUS PRÊT?" : "PRÊT");
    m_readyBtn.setPosition({ readyBtnPos.x + readyBtnSize.x * 0.5f, readyBtnPos.y + readyBtnSize.y * 0.5f });
    target.draw(m_readyBtn);

    gf::Text readyState;
    readyState.setFont(font);
    readyState.setCharacterSize(20);
    readyState.setColor(gf::Color::White);
    readyState.setString(amReady ? "Vous êtes : PRÊT" : "Vous êtes : PAS PRÊT");
    readyState.setPosition({margin, 380.f});
    target.draw(readyState);

    target.display();
}
