#include "LobbyEntity.h"

#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Anchor.h>
#include <gf/Text.h>
#include <gf/Shapes.h>
#include <gf/Sprite.h>
#include <gf/Texture.h>
#include <gf/Log.h>

LobbyEntity::LobbyEntity()
: m_font("../common/fonts/arial.ttf")
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
    m_leaveBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::Leave; 
    });
    
    m_minusBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::RoomDec; 
    });
    
    m_plusBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::RoomInc; 
    });
    
    m_minusBotBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::BotDec; 
    });
    
    m_plusBotBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::BotInc; 
    });
    
    m_minusDurBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::DurDec; 
    });
    
    m_plusDurBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::DurInc; 
    });
    
    m_readyBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::ToggleReady; 
    });
    
    m_changeRoleBtn.setCallback([this]() { 
        m_lastAction = LobbyAction::ChangeRole; 
    });

    m_container.addWidget(m_leaveBtn);
    m_container.addWidget(m_minusBtn);
    m_container.addWidget(m_plusBtn);
    m_container.addWidget(m_minusBotBtn);
    m_container.addWidget(m_plusBotBtn);
    m_container.addWidget(m_minusDurBtn);
    m_container.addWidget(m_plusDurBtn);
    m_container.addWidget(m_readyBtn);
    m_container.addWidget(m_changeRoleBtn);
}

void LobbyEntity::pointTo(gf::Vector2f coords) {
    m_container.pointTo(coords);
}

void LobbyEntity::triggerAction() {
    m_container.triggerAction();
}

void LobbyEntity::setPlayers(const std::vector<PlayerData>& players) {
    m_players = players;
}

void LobbyEntity::setRoomSettings(const RoomSettings& settings) {
    m_roomSettings = settings;
}

void LobbyEntity::setClientId(uint32_t id) {
    m_clientId = id;
}

LobbyAction LobbyEntity::getAndResetLastAction(){
    LobbyAction a = m_lastAction;
    m_lastAction = LobbyAction::None;
    return a;
}

void LobbyEntity::render(gf::RenderTarget& target, const gf::RenderStates& states) {
    // ✅ Zone logique fixe
    const float LOGICAL_W = 1280.f;
    const float LOGICAL_H = 720.f;
    const float margin = 16.f;

    // Calculs logiques pour UI
    float uiOffsetX = LOGICAL_W * 0.05f;
    float uiOffsetY = LOGICAL_H * 0.05f;

    float textPosX = margin + uiOffsetX;
    float textStartPosY = margin + uiOffsetY;

    gf::Vector2f settingsPos{textPosX, textStartPosY};
    gf::Vector2f playerListPos{textPosX, LOGICAL_H * 0.5f};
    gf::Vector2f changeRolePos{LOGICAL_W - (margin + uiOffsetX) - 120.f, textStartPosY + 24.f};
    gf::Vector2f readyPos{changeRolePos.x + 50.f, LOGICAL_H * 0.75f};

    //draws
    renderSettings(target, states, settingsPos);
    renderPlayerList(target, states, playerListPos);
    renderRoleSelection(target, states, changeRolePos);

    //get client data
    PlayerData clientData;
    for (const auto& p : m_players) {
        if (p.id == m_clientId) {
            clientData = p;
            break;
        }
    }

    const unsigned int READY_TEXT_SIZE = 18u;

    m_readyBtn.setCharacterSize(READY_TEXT_SIZE);
    m_readyBtn.setAnchor(gf::Anchor::TopCenter);
    m_readyBtn.setString(clientData.ready ? "PLUS PRÊT?" : "PRÊT");
    m_readyBtn.setPosition({readyPos.x, readyPos.y});
    defaultButtonColor(m_readyBtn);
    m_readyBtn.setDefaultBackgroundColor(clientData.ready ? gf::Color::Red : gf::Color::Green);
    m_readyBtn.setPadding(READY_TEXT_SIZE * .5f);
    target.draw(m_readyBtn, states);

    m_leaveBtn.setCharacterSize(READY_TEXT_SIZE);
    m_leaveBtn.setAnchor(gf::Anchor::TopCenter);
    m_leaveBtn.setPosition({readyPos.x, readyPos.y + 50.f});
    defaultButtonColor(m_leaveBtn);
    m_leaveBtn.setDefaultBackgroundColor(gf::Color::Red);
    m_leaveBtn.setPadding(READY_TEXT_SIZE * .5f);
    target.draw(m_leaveBtn, states);
}

void LobbyEntity::renderPlayerRow(gf::RenderTarget& target, const gf::RenderStates& states, gf::Vector2f position, const PlayerData& data) {
    const float LOGICAL_W = 1280.f;
    const float ICON_SIZE = 16.f; //taille logique avant scale
    const float SPRITE_SCALE = 4.f;

    gf::Texture iconTexture;
    if (data.role == PlayerRole::PacMan) {
        iconTexture = gf::Texture("../client/assets/pacman/pacman_icon.png");
    } else {
        iconTexture = gf::Texture("../client/assets/ghosts/inky.png");
    }

    gf::Sprite iconSprite;
    iconSprite.setAnchor(gf::Anchor::TopLeft);
    iconSprite.setTexture(iconTexture);
    iconSprite.setPosition({position.x, position.y - ICON_SIZE});
    iconSprite.setScale(SPRITE_SCALE);
    target.draw(iconSprite, states);

    gf::Text playerText;
    playerText.setFont(m_font);
    playerText.setCharacterSize(20);
    playerText.setColor(gf::Color::White);

    std::string playerStr = data.name;
    if (data.id == m_clientId) {
        playerStr += " (vous)";
    }
    playerStr += data.ready ? " (prêt)" : " (pas prêt)";

    playerText.setString(playerStr);
    playerText.setPosition({position.x + ICON_SIZE * SPRITE_SCALE + 8.f, position.y});
    target.draw(playerText, states);
}

void LobbyEntity::renderPlayerList(gf::RenderTarget& target, const gf::RenderStates& states, gf::Vector2f position) {
    const float LOGICAL_H = 720.f;

    gf::Text playerListLabel;
    playerListLabel.setFont(m_font);
    playerListLabel.setCharacterSize(24);
    playerListLabel.setColor(gf::Color::White);
    playerListLabel.setString(
        "Liste des joueurs (" +
        std::to_string(m_players.size()) + " / " +
        std::to_string(m_roomSettings.roomSize) + ") :"
    );
    playerListLabel.setPosition(position);
    target.draw(playerListLabel, states);

    float rowSpacing = 32.f; //hauteur fixe par joueur
    for (unsigned int i = 0; i < m_players.size(); i++) {
        renderPlayerRow(target, states, {position.x, position.y + rowSpacing * (i + 1)}, m_players[i]);
    }
}


void LobbyEntity::renderRoleSelection(gf::RenderTarget& target, const gf::RenderStates& states, gf::Vector2f position) {
    const float LOGICAL_W = 1280.f;
    const float ICON_SIZE = 16.f;
    const float SPRITE_SCALE = 4.f;
    const unsigned int ROLE_TEXT_SIZE = 20u;
    const unsigned int CHANGE_ROLE_TEXT_SIZE = 18u;

    PlayerData clientData;
    for (const auto& p : m_players) {
        if (p.id == m_clientId) {
            clientData = p;
            break;
        }
    }

    //Nom
    gf::Text pseudoLabel;
    pseudoLabel.setFont(m_font);
    pseudoLabel.setCharacterSize(ROLE_TEXT_SIZE);
    pseudoLabel.setAnchor(gf::Anchor::TopCenter);
    pseudoLabel.setColor(gf::Color::White);
    pseudoLabel.setString(clientData.name);
    pseudoLabel.setPosition(position);
    target.draw(pseudoLabel, states);

    //Icon
    gf::Texture iconTexture;
    if (clientData.role == PlayerRole::PacMan) {
        iconTexture = gf::Texture("../client/assets/pacman/pacman_icon.png");
    } else {
        iconTexture = gf::Texture("../client/assets/ghosts/inky.png");
    }

    gf::Sprite iconSprite;
    iconSprite.setAnchor(gf::Anchor::TopLeft);
    iconSprite.setTexture(iconTexture);
    iconSprite.setPosition({position.x, position.y + ROLE_TEXT_SIZE});
    iconSprite.setScale(SPRITE_SCALE);
    target.draw(iconSprite, states);

    //Rôle texte
    gf::Text roleLabel;
    roleLabel.setFont(m_font);
    roleLabel.setAnchor(gf::Anchor::TopCenter);
    roleLabel.setCharacterSize(ROLE_TEXT_SIZE);
    roleLabel.setColor(gf::Color::White);
    roleLabel.setString(clientData.role == PlayerRole::PacMan ? "Rôle : Pac-Man" : "Rôle : Fantôme");
    gf::Vector2f rolePos{ position.x - (roleLabel.getString().length() * ROLE_TEXT_SIZE) / 8,
                           position.y + ROLE_TEXT_SIZE + ICON_SIZE * SPRITE_SCALE + ROLE_TEXT_SIZE * 1.5f };
    roleLabel.setPosition(rolePos);
    target.draw(roleLabel, states);

    //Bouton changement rôle
    m_changeRoleBtn.setCharacterSize(CHANGE_ROLE_TEXT_SIZE);
    m_changeRoleBtn.setPosition({rolePos.x, rolePos.y + ROLE_TEXT_SIZE + CHANGE_ROLE_TEXT_SIZE});
    m_changeRoleBtn.setAnchor(gf::Anchor::TopLeft);
    defaultButtonColor(m_changeRoleBtn);
    m_changeRoleBtn.setPadding(CHANGE_ROLE_TEXT_SIZE * 0.65f);
    target.draw(m_changeRoleBtn, states);
}

void LobbyEntity::renderSettings(gf::RenderTarget& target, const gf::RenderStates& states, gf::Vector2f position) {
    const unsigned int MINUS_SIZE = 16u;
    const unsigned int PLUS_SIZE = 14u;
    const unsigned int SETTINGS_CHARACTER_SIZE = 18u;
    const float margin = 16.f;

    //Labels
    gf::Text settingsLabel;
    settingsLabel.setFont(m_font);
    settingsLabel.setCharacterSize(24u);
    settingsLabel.setColor(gf::Color::White);
    settingsLabel.setString("Paramètre du jeu");
    settingsLabel.setPosition(position);
    target.draw(settingsLabel, states);

    gf::Vector2f maxPlayerTextPos{position.x, position.y + SETTINGS_CHARACTER_SIZE * 2.5f};
    gf::Vector2f nbBotsTextPos{position.x, maxPlayerTextPos.y + SETTINGS_CHARACTER_SIZE * 2.5f};
    gf::Vector2f durationTextPos{position.x, nbBotsTextPos.y + SETTINGS_CHARACTER_SIZE * 2.5f};

    gf::Vector2f minusBtnPos{position.x + 140.f, maxPlayerTextPos.y};
    gf::Vector2f plusBtnPos{position.x + 220.f, maxPlayerTextPos.y};
    gf::Vector2f minusBotBtnPos{position.x + 140.f, nbBotsTextPos.y};
    gf::Vector2f plusBotBtnPos{position.x + 220.f, nbBotsTextPos.y};
    gf::Vector2f minusDurBtnPos{position.x + 140.f, durationTextPos.y};
    gf::Vector2f plusDurBtnPos{position.x + 220.f, durationTextPos.y};

    //Joueurs max
    gf::Text roomLabel;
    roomLabel.setFont(m_font);
    roomLabel.setCharacterSize(SETTINGS_CHARACTER_SIZE);
    roomLabel.setColor(gf::Color::White);
    roomLabel.setString("Joueurs max :");
    roomLabel.setPosition(maxPlayerTextPos);
    target.draw(roomLabel, states);

    m_minusBtn.setCharacterSize(MINUS_SIZE);
    m_minusBtn.setPosition(minusBtnPos);
    defaultButtonColor(m_minusBtn);
    m_minusBtn.setPadding(MINUS_SIZE * .65f);
    target.draw(m_minusBtn, states);

    m_plusBtn.setCharacterSize(PLUS_SIZE);
    m_plusBtn.setPosition(plusBtnPos);
    defaultButtonColor(m_plusBtn);
    m_plusBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusBtn, states);

    gf::Text valueText;
    valueText.setFont(m_font);
    valueText.setCharacterSize(20u);
    valueText.setColor(gf::Color::White);
    valueText.setString(std::to_string(m_roomSettings.roomSize));
    valueText.setPosition({(minusBtnPos.x + plusBtnPos.x - (valueText.getString().length() * SETTINGS_CHARACTER_SIZE) / 2) / 2,
                           minusBtnPos.y - 10.f});
    target.draw(valueText, states);

    //Nb bots
    gf::Text botLabel;
    botLabel.setFont(m_font);
    botLabel.setCharacterSize(SETTINGS_CHARACTER_SIZE);
    botLabel.setColor(gf::Color::White);
    botLabel.setString("Nb de bots :");
    botLabel.setPosition(nbBotsTextPos);
    target.draw(botLabel, states);

    m_minusBotBtn.setCharacterSize(MINUS_SIZE);
    m_minusBotBtn.setPosition(minusBotBtnPos);
    defaultButtonColor(m_minusBotBtn);
    m_minusBotBtn.setPadding(MINUS_SIZE * .65f);
    target.draw(m_minusBotBtn, states);

    m_plusBotBtn.setCharacterSize(PLUS_SIZE);
    m_plusBotBtn.setPosition(plusBotBtnPos);
    defaultButtonColor(m_plusBotBtn);
    m_plusBotBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusBotBtn, states);

    valueText.setString(std::to_string(m_roomSettings.nbBot));
    valueText.setPosition({(minusBotBtnPos.x + plusBotBtnPos.x - (valueText.getString().length() * SETTINGS_CHARACTER_SIZE) / 2) / 2,
                           minusBotBtnPos.y - 10.f});
    target.draw(valueText, states);

    //Durée
    gf::Text durationLabel;
    durationLabel.setFont(m_font);
    durationLabel.setCharacterSize(SETTINGS_CHARACTER_SIZE);
    durationLabel.setColor(gf::Color::White);
    durationLabel.setString("Temps de jeu (secondes) :");
    durationLabel.setPosition(durationTextPos);
    target.draw(durationLabel, states);

    m_minusDurBtn.setCharacterSize(MINUS_SIZE);
    m_minusDurBtn.setPosition(minusDurBtnPos);
    defaultButtonColor(m_minusDurBtn);
    m_minusDurBtn.setPadding(MINUS_SIZE * .65f);
    target.draw(m_minusDurBtn, states);

    m_plusDurBtn.setCharacterSize(PLUS_SIZE);
    m_plusDurBtn.setPosition(plusDurBtnPos);
    defaultButtonColor(m_plusDurBtn);
    m_plusDurBtn.setPadding(PLUS_SIZE * .65f);
    target.draw(m_plusDurBtn, states);

    valueText.setString(std::to_string(m_roomSettings.gameDuration));
    valueText.setPosition({(minusDurBtnPos.x + plusDurBtnPos.x - (valueText.getString().length() * SETTINGS_CHARACTER_SIZE) / 2) / 2,
                           minusDurBtnPos.y + 10.f});
    target.draw(valueText, states);
}


void LobbyEntity::defaultButtonColor(gf::TextButtonWidget& w)
{
    w.setDefaultTextColor(gf::Color::White);
    w.setSelectedTextColor(gf::Color::Black);
    w.setDefaultBackgroundColor(gf::Color::Black);
    w.setSelectedBackgroundColor(gf::Color::White);
    w.setBackgroundOutlineThickness(1.f);
    w.setDefaultBackgroundOutlineColor(gf::Color::White);
    w.setSelectedBackgroundOutlineColor(gf::Color::White);
}
