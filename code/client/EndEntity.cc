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

    gf::View view = target.getView();
    gf::Vector2f center = view.getCenter();
    gf::Vector2f size = view.getSize();

    float left = center.x - size.x * 0.5f;
    float top  = center.y - size.y * 0.5f;
    float width = size.x;
    float height = size.y;

    float margin = 20.f;

    //titre
    gf::Text title;
    title.setFont(m_font);
    unsigned int titleSize = std::max(24u, (unsigned int)(height * 0.06f));
    title.setCharacterSize(titleSize);
    title.setColor(gf::Color::White);
    title.setString("Fin de Jeu");
    title.setAnchor(gf::Anchor::TopCenter);
    title.setPosition({ left + width * 0.5f, top + height * 0.08f });
    target.draw(title, states);

    //texte pour la raison
    gf::Text reasonText;
    reasonText.setFont(m_font);
    unsigned int reasonSize = std::max(14u, (unsigned int)(height * 0.03f));
    reasonText.setCharacterSize(reasonSize);
    reasonText.setColor(gf::Color::White);
    reasonText.setAnchor(gf::Anchor::TopLeft);

    std::string reasonStr;
    switch (m_endReason){
        case GameEndReason::ALL_DOT_EATEN:
            reasonStr = "Pacman a mangé toutes les pacgommes.\nPacman gagne !";
            break;
        case GameEndReason::TIME_OUT:
            reasonStr = "Le temps est écoulé.\nLes fantômes gagnent.";
            break;
        case GameEndReason::PACMAN_DEATH:
            reasonStr = "Pacman est mort trop de fois.\nLes fantômes gagnent.";
            break;
        default:
            reasonStr = "Fin de la partie.";
            break;
    }

    reasonText.setString(reasonStr);
    reasonText.setPosition({ left + width * 0.16f, top + height * 0.18f });
    target.draw(reasonText, states);

    //le score
    gf::Text scoreText;
    scoreText.setFont(m_font);
    unsigned int scoreSize = std::max(12u, (unsigned int)(height * 0.028f));
    scoreText.setCharacterSize(scoreSize);
    scoreText.setColor(gf::Color::White);
    scoreText.setAnchor(gf::Anchor::TopLeft);
    scoreText.setString("Score final de Pacman : " + std::to_string(m_lastScore));
    scoreText.setPosition({ left + width * 0.16f, top + height * 0.30f });
    target.draw(scoreText, states);

    //zone de bouton
    float bw = width * 0.4f;
    float bh = height * 0.12f;
    float bx = left + (width - bw) * 0.5f;
    float by = top + height * 0.55f;

    unsigned int btnChar = std::max(16u, (unsigned int)(height * 0.035f));

    m_enterWidget.setCharacterSize(btnChar);
    m_enterWidget.setAnchor(gf::Anchor::Center);
    m_enterWidget.setPosition({ bx + bw * 0.5f, by + bh * 0.5f });

    m_enterWidget.setDefaultTextColor(gf::Color::White);
    m_enterWidget.setSelectedTextColor(gf::Color::Black);
    m_enterWidget.setDefaultBackgroundColor(gf::Color::Black);
    m_enterWidget.setSelectedBackgroundColor(gf::Color::White);
    m_enterWidget.setBackgroundOutlineThickness(btnChar * .05f);
    m_enterWidget.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_enterWidget.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_enterWidget.setPadding(btnChar * .4f);

    target.draw(m_enterWidget, states);
}
