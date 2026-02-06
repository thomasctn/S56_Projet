#include "WelcomeEntity.h"
#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Coordinates.h>
#include <gf/Text.h>
#include <gf/Color.h>


WelcomeEntity::WelcomeEntity(gf::Font& font)
: m_enterWidget("ENTRER", font)
, m_font(font)
{
    m_enterWidget.setCallback([this]() { m_clicked = true; });
    m_container.addWidget(m_enterWidget);
}

void WelcomeEntity::pointTo(gf::Vector2f coords) {
    m_container.pointTo(coords);   
  
}

void WelcomeEntity::triggerAction() {
    m_container.triggerAction();
}

bool WelcomeEntity::wasClicked() const {
    return m_clicked;
}

void WelcomeEntity::resetClick() {
    m_clicked = false;
}

void WelcomeEntity::render(gf::RenderTarget& target, const gf::RenderStates& states) {
    const float LOGICAL_W = 1280.f;
    const float LOGICAL_H = 720.f;

    unsigned titleSize = 64u; 
    gf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(titleSize);
    title.setString("PACMAN");
    title.setAnchor(gf::Anchor::Center);
    title.setColor(gf::Color::White);
    title.setPosition({ LOGICAL_W * 0.5f, LOGICAL_H * 0.2f });
    target.draw(title, states);

    unsigned charSize = 32u;
    gf::Vector2f buttonPos{ LOGICAL_W * 0.5f, LOGICAL_H * 0.5f };

    m_enterWidget.setCharacterSize(charSize);
    m_enterWidget.setAnchor(gf::Anchor::Center);
    m_enterWidget.setPosition(buttonPos);

    m_enterWidget.setDefaultTextColor(gf::Color::White);
    m_enterWidget.setSelectedTextColor(gf::Color::Black);
    m_enterWidget.setDefaultBackgroundColor(gf::Color::Black);
    m_enterWidget.setSelectedBackgroundColor(gf::Color::White);

    target.draw(m_enterWidget, states);
}

