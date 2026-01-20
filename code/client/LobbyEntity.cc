#include "LobbyEntity.h"
#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Log.h>
#include <gf/Anchor.h>
#include <gf/Coordinates.h>

LobbyEntity::LobbyEntity(Renderer& renderer)
: m_renderer(renderer)
, m_font("../common/fonts/arial.ttf")
, m_leaveBtn("Quitter", m_font)
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
    m_leaveBtn.setCallback([this](){ m_lastAction = LobbyAction::Leave; });
    m_minusBtn.setCallback([this](){ m_lastAction = LobbyAction::RoomDec; });
    m_plusBtn.setCallback([this](){ m_lastAction = LobbyAction::RoomInc; });
    m_minusBotBtn.setCallback([this](){ m_lastAction = LobbyAction::BotDec; });
    m_plusBotBtn.setCallback([this](){ m_lastAction = LobbyAction::BotInc; });
    m_minusDurBtn.setCallback([this](){ m_lastAction = LobbyAction::DurDec; });
    m_plusDurBtn.setCallback([this](){ m_lastAction = LobbyAction::DurInc; });
    m_readyBtn.setCallback([this](){ m_lastAction = LobbyAction::ToggleReady; });
    m_changeRoleBtn.setCallback([this](){ m_lastAction = LobbyAction::ChangeRole; });

    m_container.addWidget(m_leaveBtn);
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
void LobbyEntity::render(std::vector<PlayerData> players,RoomSettings settings, uint32_t clientID) {
    gf::RenderWindow& target = m_renderer.getRenderWindow();
    m_renderer.clearWindow();

    gf::Coordinates coords(target);
    gf::View view = target.getView();
    gf::Vector2f center = view.getCenter();
    gf::Vector2f size = view.getSize();

    float left = center.x - size.x * 0.5f;
    float right = center.x + size.x * 0.5f;
    float top = center.y - size.y * 0.5f;
    float width = size.x;
    float height = size.y;
    float margin = 20.f;

    float uiOffsetX = width * 0.05f;
    float uiOffsetY = height * 0.05f;

    float textPosX = left + margin +uiOffsetX;
    float textStartPosY = uiOffsetY + margin;

    gf::Vector2f settingsPos{textPosX, textStartPosY};
    gf::Vector2f playerListPos{textPosX, top + height * 0.5f};
    gf::Vector2f changeRolePos{right - (margin +uiOffsetX) - 120.f, textStartPosY+24.f};
    gf::Vector2f readyPos{changeRolePos.x+50.f, top + height * 0.75f};
    PlayerData clientData;
    for (auto& p : players) {
        if (p.id == clientID) {
            clientData = p;
            break;
        }
    }

    renderPLayerList(playerListPos,players,settings.roomSize, clientID);
    renderSettings(settingsPos,settings);
    renderRoleSelection(changeRolePos,clientData);

    const unsigned int READY_TEXT_SIZE  = 18u;

    m_readyBtn.setCharacterSize(READY_TEXT_SIZE);
    m_readyBtn.setAnchor(gf::Anchor::TopCenter);
    m_readyBtn.setString(clientData.ready ? "PLUS PRÊT?" : "PRÊT");
    m_readyBtn.setPosition({readyPos.x,readyPos.y});
    m_readyBtn.setDefaultTextColor(gf::Color::White);
    m_readyBtn.setSelectedTextColor(gf::Color::Black);
    m_readyBtn.setDefaultBackgroundColor(clientData.ready ? gf::Color::Red : gf::Color::Green);
    m_readyBtn.setSelectedBackgroundColor(gf::Color::White);
    m_readyBtn.setBackgroundOutlineThickness(1.f);
    m_readyBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_readyBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_readyBtn.setPadding(READY_TEXT_SIZE *.5f);
    target.draw(m_readyBtn);

    m_leaveBtn.setCharacterSize(READY_TEXT_SIZE);
    m_leaveBtn.setAnchor(gf::Anchor::TopCenter);
    m_leaveBtn.setPosition({readyPos.x,readyPos.y + 50.f});
    m_leaveBtn.setDefaultTextColor(gf::Color::White);
    m_leaveBtn.setSelectedTextColor(gf::Color::Black);
    m_leaveBtn.setDefaultBackgroundColor(gf::Color::Red);
    m_leaveBtn.setSelectedBackgroundColor(gf::Color::White);
    m_leaveBtn.setBackgroundOutlineThickness(1.f);
    m_leaveBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_leaveBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_leaveBtn.setPadding(READY_TEXT_SIZE *.5f);
    target.draw(m_leaveBtn);

    target.display();
}

void LobbyEntity::renderPlayerRow(gf::Vector2f position, PlayerData data, uint32_t clientID)
{
    gf::RenderWindow& target = m_renderer.getRenderWindow();
    gf::Sprite iconSprite;
    gf::Texture iconTexture;
    if(data.role == PlayerRole::PacMan) {
        iconTexture = gf::Texture("../client/assets/pacman/pacman_icon.png");
    }
    else {
        iconTexture = gf::Texture("../client/assets/ghosts/inky.png");
    }
    iconSprite.setAnchor(gf::Anchor::TopLeft);
    iconSprite.setTexture(iconTexture);
    iconSprite.setPosition({position.x,position.y-14.f});

    gf::Text playerListLabel;
    playerListLabel.setFont(m_font);
    playerListLabel.setCharacterSize(20);
    playerListLabel.setColor(gf::Color::White);
    playerListLabel.setString(data.name + (clientID == data.id ? "(vous)" : "") + (data.ready ? "(prêt)" : "(pas prêt)"));
    playerListLabel.setPosition({position.x + 32.f, position.y});
    target.draw(iconSprite);
    target.draw(playerListLabel);
}

void LobbyEntity::renderPLayerList(gf::Vector2f position, std::vector<PlayerData> players, unsigned int roomSize, uint32_t clientID)
{
    gf::RenderWindow& target = m_renderer.getRenderWindow();

    gf::Text playerListLabel;
    playerListLabel.setFont(m_font);
    playerListLabel.setCharacterSize(24);
    playerListLabel.setColor(gf::Color::White);
    playerListLabel.setString("Liste des joueurs ("+std::to_string(players.size()) + " / " + std::to_string(roomSize) +") :");
    playerListLabel.setPosition(position);
    target.draw(playerListLabel);
    for(unsigned int i =0; i < players.size(); i++)
    {
        renderPlayerRow({position.x, position.y + 32.f*(i+1)},players.at(i), clientID);
    }

}

void LobbyEntity::renderRoleSelection(gf::Vector2f position, PlayerData clientData)
{
    gf::RenderWindow& target = m_renderer.getRenderWindow();



    const unsigned int TEXTURE_SIZE = 16u;
    const float SPRITE_SCALE_FACTOR = 4.f;
    const unsigned int ROLE_TEXT_SIZE = 20u;
    const unsigned int CHANGE_ROLE_TEXT_SIZE = 18u;

    gf::Text pseudoLabel;
    pseudoLabel.setFont(m_font);
    pseudoLabel.setAnchor(gf::Anchor::TopCenter);
    pseudoLabel.setCharacterSize(ROLE_TEXT_SIZE);
    pseudoLabel.setColor(gf::Color::White);
    pseudoLabel.setString(clientData.name);
    pseudoLabel.setPosition(position);
    target.draw(pseudoLabel);

    gf::Sprite iconSprite;
    gf::Texture iconTexture;
    if(clientData.role == PlayerRole::PacMan) {
        iconTexture = gf::Texture("../client/assets/pacman/pacman_icon.png");
    }
    else {
        iconTexture = gf::Texture("../client/assets/ghosts/inky.png");
    }
    iconSprite.setAnchor(gf::Anchor::TopLeft);
    iconSprite.setTexture(iconTexture);

    gf::Vector2f spritePos = {position.x,position.y+ROLE_TEXT_SIZE};
    iconSprite.setPosition(spritePos);
    iconSprite.setScale(SPRITE_SCALE_FACTOR);
    target.draw(iconSprite);

    gf::Text roleLabel;
    roleLabel.setFont(m_font);
    roleLabel.setAnchor(gf::Anchor::TopLeft);
    roleLabel.setCharacterSize(ROLE_TEXT_SIZE);
    roleLabel.setColor(gf::Color::White);
    roleLabel.setString((clientData.role == PlayerRole::PacMan ? "Rôle : Pac-Man" : "Rôle : Fantôme"));
    gf::Vector2f rolePos{position.x - (roleLabel.getString().length() * ROLE_TEXT_SIZE) /8,spritePos.y + (TEXTURE_SIZE * SPRITE_SCALE_FACTOR)+ ROLE_TEXT_SIZE*3};
    roleLabel.setPosition(rolePos);
    target.draw(roleLabel);


    m_changeRoleBtn.setCharacterSize(CHANGE_ROLE_TEXT_SIZE);

    gf::Vector2f changeRoleBtnPos{position.x, rolePos.y + ROLE_TEXT_SIZE + CHANGE_ROLE_TEXT_SIZE};


    m_changeRoleBtn.setPosition(changeRoleBtnPos);
    m_changeRoleBtn.setDefaultTextColor(gf::Color::White);
    m_changeRoleBtn.setSelectedTextColor(gf::Color::Black);
    m_changeRoleBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_changeRoleBtn.setSelectedBackgroundColor(gf::Color::White);
    m_changeRoleBtn.setBackgroundOutlineThickness(CHANGE_ROLE_TEXT_SIZE* .05f);
    m_changeRoleBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_changeRoleBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_changeRoleBtn.setPadding(CHANGE_ROLE_TEXT_SIZE * .65f);
    m_changeRoleBtn.setAnchor(gf::Anchor::TopLeft);
    target.draw(m_changeRoleBtn);
}

void LobbyEntity::renderSettings(gf::Vector2f position, RoomSettings settings)
{
    gf::RenderWindow& target = m_renderer.getRenderWindow();

    const unsigned int MINUS_SIZE = 16u;
    const unsigned int PLUS_SIZE = 14u;
    const unsigned int SETTINGS_CHARACTER_SIZE = 18u;

    float btnPosX = position.x + 140.f;
    float btnPosX2 = btnPosX + 80.f;
    gf::Vector2f settingsTextPos{position.x, position.y};
    gf::Vector2f maxPlayerTextPos{position.x, settingsTextPos.y + SETTINGS_CHARACTER_SIZE*2.5f};
    gf::Vector2f nbBotsTextPos{position.x, maxPlayerTextPos.y + SETTINGS_CHARACTER_SIZE*2.5f};
    gf::Vector2f durationTextPos{position.x, nbBotsTextPos.y + SETTINGS_CHARACTER_SIZE*2.5f};

    gf::Vector2f minusBtnPos{btnPosX, maxPlayerTextPos.y};
    gf::Vector2f plusBtnPos{btnPosX2, maxPlayerTextPos.y};
    gf::Vector2f minusBotBtnPos{btnPosX, nbBotsTextPos.y};
    gf::Vector2f plusBotBtnPos{btnPosX2, nbBotsTextPos.y};
    gf::Vector2f minusDurBtnPos{btnPosX, durationTextPos.y};
    gf::Vector2f plusDurBtnPos{btnPosX2, durationTextPos.y};

    gf::Text settingsLabel;
    settingsLabel.setFont(m_font);
    settingsLabel.setCharacterSize(24u);
    settingsLabel.setColor(gf::Color::White);
    settingsLabel.setString("Paramètre du jeu");
    settingsLabel.setPosition(settingsTextPos);
    target.draw(settingsLabel);


    gf::Text roomLabel;
    roomLabel.setFont(m_font);
    roomLabel.setCharacterSize(SETTINGS_CHARACTER_SIZE);
    roomLabel.setColor(gf::Color::White);
    roomLabel.setString("Joueurs max :");
    roomLabel.setPosition(maxPlayerTextPos);
    target.draw(roomLabel);

    m_minusBtn.setCharacterSize(MINUS_SIZE);
    m_minusBtn.setAnchor(gf::Anchor::CenterLeft);
    m_minusBtn.setPosition(minusBtnPos);

    m_minusBtn.setDefaultTextColor(gf::Color::White);
    m_minusBtn.setSelectedTextColor(gf::Color::Black);
    m_minusBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_minusBtn.setSelectedBackgroundColor(gf::Color::White);
    m_minusBtn.setBackgroundOutlineThickness(1.f);
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
    m_plusBtn.setBackgroundOutlineThickness(1.f);
    m_plusBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_plusBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_plusBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusBtn);

    gf::Text valueText;
    valueText.setFont(m_font);
    valueText.setCharacterSize(20u);
    valueText.setColor(gf::Color::White);
    valueText.setString(std::to_string(settings.roomSize));
    valueText.setPosition({(minusBtnPos.x + plusBtnPos.x - (valueText.getString().length() * SETTINGS_CHARACTER_SIZE) / 2) / 2, minusBtnPos.y + 20U /2});
    target.draw(valueText);

    gf::Text botLabel;
    botLabel.setFont(m_font);
    botLabel.setCharacterSize(SETTINGS_CHARACTER_SIZE);
    botLabel.setColor(gf::Color::White);
    botLabel.setString("Nb de bots :");
    botLabel.setPosition(nbBotsTextPos);
    target.draw(botLabel);

    m_minusBotBtn.setCharacterSize(MINUS_SIZE);
    m_minusBotBtn.setAnchor(gf::Anchor::CenterLeft);
    m_minusBotBtn.setPosition(minusBotBtnPos);
    m_minusBotBtn.setDefaultTextColor(gf::Color::White);
    m_minusBotBtn.setSelectedTextColor(gf::Color::Black);
    m_minusBotBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_minusBotBtn.setSelectedBackgroundColor(gf::Color::White);
    m_minusBotBtn.setBackgroundOutlineThickness(1.f);
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
    m_plusBotBtn.setBackgroundOutlineThickness(1.f);
    m_plusBotBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_plusBotBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_plusBotBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusBotBtn);

    valueText.setString(std::to_string(settings.nbBot));
    valueText.setPosition({(minusBotBtnPos.x + plusBotBtnPos.x - (valueText.getString().length() * SETTINGS_CHARACTER_SIZE) / 2) / 2, minusBotBtnPos.y + 20U /2});
    target.draw(valueText);

    gf::Text durationLabel;
    durationLabel.setFont(m_font);
    durationLabel.setCharacterSize(SETTINGS_CHARACTER_SIZE);
    durationLabel.setColor(gf::Color::White);
    durationLabel.setString("Temps de jeu\n(secondes) :");
    durationLabel.setPosition(durationTextPos);
    target.draw(durationLabel);

    m_minusDurBtn.setCharacterSize(MINUS_SIZE);
    m_minusDurBtn.setAnchor(gf::Anchor::CenterLeft);
    m_minusDurBtn.setPosition(minusDurBtnPos);
    m_minusDurBtn.setDefaultTextColor(gf::Color::White);
    m_minusDurBtn.setSelectedTextColor(gf::Color::Black);
    m_minusDurBtn.setDefaultBackgroundColor(gf::Color::Black);
    m_minusDurBtn.setSelectedBackgroundColor(gf::Color::White);
    m_minusDurBtn.setBackgroundOutlineThickness(1.f);
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
    m_plusDurBtn.setBackgroundOutlineThickness(1.f);
    m_plusDurBtn.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_plusDurBtn.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_plusDurBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusDurBtn);

    valueText.setString(std::to_string(settings.gameDuration));
    valueText.setPosition({(minusDurBtnPos.x + plusDurBtnPos.x - (valueText.getString().length() * SETTINGS_CHARACTER_SIZE) / 2) / 2, minusDurBtnPos.y + 20U /2});
    target.draw(valueText);
}
