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
    gf::Coordinates coords(target);
    

    unsigned titleSize = coords.getRelativeCharacterSize(0.15f);
    auto titlePos = coords.getRelativePoint({0.5f, 0.2f});
    gf::Text title("PACMAN", m_font, titleSize);
    title.setAnchor(gf::Anchor::Center);
    title.setColor(gf::Color::White);
    title.setPosition(titlePos);
    target.draw(title, states);

    unsigned charSize = coords.getRelativeCharacterSize(0.1f);
    auto buttonPos = coords.getRelativePoint({0.5f, 0.5f});

    m_enterWidget.setCharacterSize(charSize);
    m_enterWidget.setAnchor(gf::Anchor::Center);
    m_enterWidget.setPosition(buttonPos);

    m_enterWidget.setDefaultTextColor(gf::Color::White);
    m_enterWidget.setSelectedTextColor(gf::Color::Black);
    m_enterWidget.setDefaultBackgroundColor(gf::Color::Black);
    m_enterWidget.setSelectedBackgroundColor(gf::Color::White);

    target.draw(m_enterWidget, states);
}
