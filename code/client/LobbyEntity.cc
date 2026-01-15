#include "LobbyEntity.h"
#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Log.h>
#include <gf/Anchor.h>
#include <gf/Coordinates.h>

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

LobbyAction LobbyEntity::processEvent(const gf::Event& event) {
    gf::RenderWindow& win = m_renderer.getRenderWindow();

    switch (event.type) {
        case gf::EventType::MouseMoved: {
            // Convertit automatiquement la position de la souris en coordonnées logiques de la fenêtre
            m_container.pointTo(win.mapPixelToCoords({ int(event.mouseCursor.coords.x), int(event.mouseCursor.coords.y) }));
            return LobbyAction::None;
        }

        case gf::EventType::MouseButtonPressed: {
            if (event.mouseButton.button != gf::MouseButton::Left)
                return LobbyAction::None;

            m_container.pointTo(win.mapPixelToCoords({ int(event.mouseButton.coords.x), int(event.mouseButton.coords.y) }));
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

    gf::Coordinates coords(target);
    gf::View view = target.getView();
    gf::Vector2f center = view.getCenter();
    gf::Vector2f size = view.getSize();

    float left = center.x - size.x * 0.5f;
    float top = center.y - size.y * 0.5f;
    float width = size.x;
    float height = size.y;
    float margin = 20.f;

    float uiOffsetX = width * 0.1f;

    float textPosX = left + margin +uiOffsetX;
    float textStartPosY = 100.f;

    float btnPosX = textPosX + 180.f;
    float btnPosX2 = btnPosX + 100.f;

    unsigned int MINUS_SIZE = 24u;
    unsigned int PLUS_SIZE = 20u;

    gf::Vector2f minusBtnPos{btnPosX, textStartPosY};
    gf::Vector2f plusBtnPos{btnPosX2, minusBtnPos.y};
    gf::Vector2f minusBotBtnPos{btnPosX, textStartPosY + 57.5f};
    gf::Vector2f plusBotBtnPos{btnPosX2, minusBotBtnPos.y};
    gf::Vector2f minusDurBtnPos{btnPosX, minusBotBtnPos.y + 57.5f};
    gf::Vector2f plusDurBtnPos{btnPosX2, minusDurBtnPos.y};
    gf::Vector2f changeRolePos{btnPosX, minusBotBtnPos.y + 125.f};
    gf::Vector2f readyBtnPos{textPosX, top + height * 0.75f};

    static gf::Font font("../common/fonts/arial.ttf");

    gf::Text title;
    title.setFont(font);
    title.setCharacterSize(28);
    title.setColor(gf::Color::White);
    title.setString("En attente de joueurs...");
    title.setPosition({left + margin +uiOffsetX, top + 20.f});
    target.draw(title);

    gf::Text countText;
    countText.setFont(font);
    countText.setCharacterSize(20);
    countText.setColor(gf::Color::White);
    countText.setString(std::to_string(connectedPlayers) + " / " + std::to_string(roomSize));
    countText.setPosition({left + margin +uiOffsetX, top + 60.f});
    target.draw(countText);

    gf::Text roomLabel;
    roomLabel.setFont(font);
    roomLabel.setCharacterSize(20);
    roomLabel.setColor(gf::Color::White);
    roomLabel.setString("Joueurs max :");
    roomLabel.setPosition({textPosX, minusBtnPos.y});
    target.draw(roomLabel);

    m_minusBtn.setCharacterSize(MINUS_SIZE);
    m_minusBtn.setAnchor(gf::Anchor::CenterLeft);
    m_minusBtn.setPosition(minusBtnPos);

    m_minusBtn.setDefaultTextColor(gf::Color::White);
    m_minusBtn.setSelectedTextColor(gf::Color::Black);
    m_minusBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_minusBtn.setSelectedBackgroundColor(gf::Color::White);
    m_minusBtn.setBackgroundOutlineThickness(MINUS_SIZE * .05f);
    m_minusBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_minusBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_minusBtn.setPadding(MINUS_SIZE * .65f);
    target.draw(m_minusBtn);

    m_plusBtn.setCharacterSize(PLUS_SIZE);
    m_plusBtn.setAnchor(gf::Anchor::CenterLeft);
    m_plusBtn.setPosition(plusBtnPos);
    m_plusBtn.setDefaultTextColor(gf::Color::White);
    m_plusBtn.setSelectedTextColor(gf::Color::Black);
    m_plusBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_plusBtn.setSelectedBackgroundColor(gf::Color::White);
    m_plusBtn.setBackgroundOutlineThickness(MINUS_SIZE * .05f);
    m_plusBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_plusBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_plusBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusBtn);

    gf::Text valueText;
    valueText.setFont(font);
    valueText.setCharacterSize(20);
    valueText.setColor(gf::Color::White);
    valueText.setString(std::to_string(roomSize));
    valueText.setPosition({(minusBtnPos.x + plusBtnPos.x - (valueText.getString().length() * 20U) / 2) / 2, minusBtnPos.y + 20U /2});
    target.draw(valueText);

    gf::Text botLabel;
    botLabel.setFont(font);
    botLabel.setCharacterSize(20);
    botLabel.setColor(gf::Color::White);
    botLabel.setString("Nb de bots :");
    botLabel.setPosition({textPosX, minusBotBtnPos.y});
    target.draw(botLabel);

    m_minusBotBtn.setCharacterSize(24);
    m_minusBotBtn.setAnchor(gf::Anchor::CenterLeft);
    m_minusBotBtn.setPosition(minusBotBtnPos);
    m_minusBotBtn.setDefaultTextColor(gf::Color::White);
    m_minusBotBtn.setSelectedTextColor(gf::Color::Black);
    m_minusBotBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_minusBotBtn.setSelectedBackgroundColor(gf::Color::White);
    m_minusBotBtn.setBackgroundOutlineThickness(MINUS_SIZE * .05f);
    m_minusBotBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_minusBotBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_minusBotBtn.setPadding(MINUS_SIZE * .65f);
    target.draw(m_minusBotBtn);

    m_plusBotBtn.setCharacterSize(PLUS_SIZE);
    m_plusBotBtn.setAnchor(gf::Anchor::CenterLeft);
    m_plusBotBtn.setPosition(plusBotBtnPos);
    m_plusBotBtn.setDefaultTextColor(gf::Color::White);
    m_plusBotBtn.setSelectedTextColor(gf::Color::Black);
    m_plusBotBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_plusBotBtn.setSelectedBackgroundColor(gf::Color::White);
    m_plusBotBtn.setBackgroundOutlineThickness(MINUS_SIZE * .05f);
    m_plusBotBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_plusBotBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_plusBotBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusBotBtn);

    valueText.setString(std::to_string(nbBots));
    valueText.setPosition({(minusBotBtnPos.x + plusBotBtnPos.x - (valueText.getString().length() * 20U) / 2) / 2, minusBotBtnPos.y + 20U /2});
    target.draw(valueText);

    gf::Text durationLabel;
    durationLabel.setFont(font);
    durationLabel.setCharacterSize(20);
    durationLabel.setColor(gf::Color::White);
    durationLabel.setString("Temps de jeu\n(secondes) :");
    durationLabel.setPosition({textPosX, minusDurBtnPos.y});
    target.draw(durationLabel);

    m_minusDurBtn.setCharacterSize(24);
    m_minusDurBtn.setAnchor(gf::Anchor::CenterLeft);
    m_minusDurBtn.setPosition(minusDurBtnPos);
    m_minusDurBtn.setDefaultTextColor(gf::Color::White);
    m_minusDurBtn.setSelectedTextColor(gf::Color::Black);
    m_minusDurBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_minusDurBtn.setSelectedBackgroundColor(gf::Color::White);
    m_minusDurBtn.setBackgroundOutlineThickness(MINUS_SIZE * .05f);
    m_minusDurBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_minusDurBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_minusDurBtn.setPadding(MINUS_SIZE * .65f);
    target.draw(m_minusDurBtn);

    m_plusDurBtn.setCharacterSize(PLUS_SIZE);
    m_plusDurBtn.setAnchor(gf::Anchor::CenterLeft);
    m_plusDurBtn.setPosition(plusDurBtnPos);
    m_plusDurBtn.setDefaultTextColor(gf::Color::White);
    m_plusDurBtn.setSelectedTextColor(gf::Color::Black);
    m_plusDurBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_plusDurBtn.setSelectedBackgroundColor(gf::Color::White);
    m_plusDurBtn.setBackgroundOutlineThickness(MINUS_SIZE * .05f);
    m_plusDurBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_plusDurBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_plusDurBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusDurBtn);

    valueText.setString(std::to_string(gameDur));
    valueText.setPosition({(minusDurBtnPos.x + plusDurBtnPos.x - (valueText.getString().length() * 20U) / 2) / 2, minusDurBtnPos.y + 20U /2});
    target.draw(valueText);

    gf::Text roleLabel;
    roleLabel.setFont(font);
    roleLabel.setCharacterSize(20);
    roleLabel.setColor(gf::Color::White);
    roleLabel.setString(myRole == PlayerRole::PacMan ? "Vous etes\nactuellement :\nPACMAN" : "Vous etes\nactuellement :\nUN FANTOME");
    roleLabel.setPosition({textPosX, changeRolePos.y});
    target.draw(roleLabel);

    m_changeRoleBtn.setCharacterSize(18U);
    m_changeRoleBtn.setAnchor(gf::Anchor::CenterLeft);
    m_changeRoleBtn.setString(myRole == PlayerRole::PacMan ? "Devenir un fantome ?" : "Devenir Pacman ?");
    m_changeRoleBtn.setPosition(changeRolePos);
    m_changeRoleBtn.setDefaultTextColor(gf::Color::White);
    m_changeRoleBtn.setSelectedTextColor(gf::Color::Black);
    m_changeRoleBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_changeRoleBtn.setSelectedBackgroundColor(gf::Color::White);
    m_changeRoleBtn.setBackgroundOutlineThickness(18U * .05f);
    m_changeRoleBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_changeRoleBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_changeRoleBtn.setPadding(18U * .65f);
    target.draw(m_changeRoleBtn);


    m_readyBtn.setCharacterSize(24U);
    m_readyBtn.setAnchor(gf::Anchor::CenterLeft);
    m_readyBtn.setString(amReady ? "PLUS PRÊT?" : "PRÊT");
    m_readyBtn.setPosition(readyBtnPos);
    m_readyBtn.setDefaultTextColor(gf::Color::White);
    m_readyBtn.setSelectedTextColor(gf::Color::Black);
    m_readyBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_readyBtn.setSelectedBackgroundColor(gf::Color::White);
    m_readyBtn.setBackgroundOutlineThickness(24U * .05f);
    m_readyBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_readyBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_readyBtn.setPadding(24U *.5f);
    target.draw(m_readyBtn);

    gf::Text readyState;
    readyState.setFont(font);
    readyState.setCharacterSize(20);
    readyState.setColor(gf::Color::White);
    readyState.setString(amReady ? "Vous êtes : PRÊT" : "Vous êtes : PAS PRÊT");
    readyState.setPosition({left + margin +uiOffsetX, top + 380.f});
    target.draw(readyState);

    target.display();
}
