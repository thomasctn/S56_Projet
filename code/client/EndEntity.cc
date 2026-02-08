#include "EndEntity.h"

#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Anchor.h>
#include <gf/Text.h>
#include <gf/Shapes.h>
#include <gf/Log.h>

EndEntity::EndEntity()
: m_font("../common/fonts/arial.ttf")
, m_enterWidget("Retour au Lobby", m_font)
{
    m_enterWidget.setCallback([this](){
        m_lastAction = EndAction::BackToLobby;
    });
    m_container.addWidget(m_enterWidget);
    m_enterWidget.setCharacterSize(24);
    m_enterWidget.setAnchor(gf::Anchor::Center);
}

void EndEntity::pointTo(gf::Vector2f position) {
    m_container.pointTo(position);
}

void EndEntity::triggerAction() {
    m_container.triggerAction();
}

void EndEntity::setEndData(GameEndReason reason, int lastScore) {
    m_endReason = reason;
    m_lastScore = lastScore;
}

EndAction EndEntity::getAndResetLastAction() {
    EndAction a = m_lastAction;
    m_lastAction = EndAction::None;
    return a;
}

void EndEntity::render(gf::RenderTarget& target, const gf::RenderStates& states) {
    const float LOGICAL_W = 1280.f;
    const float LOGICAL_H = 720.f;

    unsigned int titleSize = 64u;
    gf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(titleSize);
    title.setString("Fin de Jeu");
    title.setAnchor(gf::Anchor::Center);
    title.setColor(gf::Color::White);
    title.setPosition({ LOGICAL_W * 0.5f, LOGICAL_H * 0.15f });
    target.draw(title, states);

    unsigned int reasonSize = 32u;
    gf::Text reasonText;
    reasonText.setFont(m_font);
    reasonText.setCharacterSize(reasonSize);
    reasonText.setColor(gf::Color::White);
    reasonText.setAnchor(gf::Anchor::TopLeft);

    std::string reasonStr;
    switch (m_endReason) {
        case GameEndReason::ALL_DOT_EATEN: 
            reasonStr = "Pacman a mangé toutes les pacgommes.\nPacman gagne !"; break;
        case GameEndReason::TIME_OUT:       
            reasonStr = "Le temps est écoulé.\nLes fantômes gagnent."; break;
        case GameEndReason::PACMAN_DEATH:   
            reasonStr = "Pacman est mort trop de fois.\nLes fantômes gagnent."; break;
        default:                            
            reasonStr = "Fin de la partie."; break;
    }

    reasonText.setString(reasonStr);
    reasonText.setPosition({ LOGICAL_W * 0.1f, LOGICAL_H * 0.25f });
    target.draw(reasonText, states);

    unsigned int scoreSize = 28u;
    gf::Text scoreText;
    scoreText.setFont(m_font);
    scoreText.setCharacterSize(scoreSize);
    scoreText.setColor(gf::Color::White);
    scoreText.setAnchor(gf::Anchor::TopLeft);
    scoreText.setString("Score final de Pacman : " + std::to_string(m_lastScore));
    scoreText.setPosition({ LOGICAL_W * 0.1f, LOGICAL_H * 0.40f });
    target.draw(scoreText, states);

    float bw = 400.f; // largeur du bouton
    float bh = 80.f;  // hauteur du bouton
    float bx = (LOGICAL_W - bw) * 0.5f;
    float by = LOGICAL_H * 0.60f;

    unsigned int btnChar = 32u;

    m_enterWidget.setCharacterSize(btnChar);
    m_enterWidget.setAnchor(gf::Anchor::Center);
    m_enterWidget.setPosition({ bx + bw * 0.5f, by + bh * 0.5f });

    m_enterWidget.setDefaultTextColor(gf::Color::White);
    m_enterWidget.setSelectedTextColor(gf::Color::Black);
    m_enterWidget.setDefaultBackgroundColor(gf::Color::Black);
    m_enterWidget.setSelectedBackgroundColor(gf::Color::White);
    m_enterWidget.setBackgroundOutlineThickness(btnChar * 0.05f);
    m_enterWidget.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_enterWidget.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_enterWidget.setPadding(btnChar * 0.4f);

    target.draw(m_enterWidget, states);
}
