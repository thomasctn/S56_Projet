#include "WelcomeEntity.h"
#include <gf/Coordinates.h>
#include <gf/RenderStates.h>

WelcomeEntity::WelcomeEntity(Renderer& renderer): m_renderer(renderer), m_font("../common/fonts/arial.ttf"), m_enterWidget("ENTRER", m_font), m_wasClicked(false){
    m_enterWidget.setCallback([this](){
        m_wasClicked = true;
    });
    m_container.addWidget(m_enterWidget);
    m_enterWidget.setCharacterSize(24);
    m_enterWidget.setAnchor(gf::Anchor::Center);
}

bool WelcomeEntity::wasClicked() const {
    return m_wasClicked;
}

void WelcomeEntity::resetClick() {
    m_wasClicked = false;
}

 
void WelcomeEntity::pointTo(gf::Vector2f coords) {
    m_container.pointTo(coords);
}

bool WelcomeEntity::trigger() {
    m_container.triggerAction();
    if (m_wasClicked) {
        m_wasClicked = false;
        return true;
    }
    return false;
}



void WelcomeEntity::render(gf::RenderTarget& target) {
   /* m_renderer.clearWindow();
    gf::RenderWindow& target = m_renderer.getRenderWindow();*/

    gf::View view = target.getView();
    gf::Vector2f center= view.getCenter();
    gf::Vector2f size = view.getSize();

    float left= center.x - size.x * 0.5f;
    float top= center.y - size.y * 0.5f;
    float width = size.x;
    float height = size.y;

    //titre
    gf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(48);
    title.setColor(gf::Color::White);
    title.setString("PACMAN");
    title.setAnchor(gf::Anchor::TopCenter);
    title.setPosition({ left + width * 0.5f, top + height * 0.1f });
    target.draw(title);

    //bouton
    float bw = width * 0.4f;
    float bh = height * 0.12f;
    float bx = left + (width - bw) * 0.5f;
    float by = top+ height * 0.45f;

    m_enterWidget.setCharacterSize(26);
    m_enterWidget.setAnchor(gf::Anchor::Center);
    m_enterWidget.setPosition({ bx + bw * 0.5f, by + bh * 0.5f });
    m_enterWidget.setDefaultTextColor(gf::Color::White);
    m_enterWidget.setSelectedTextColor(gf::Color::Black);
    m_enterWidget.setDefaultBackgroundColor(gf::Color::Black);
    m_enterWidget.setSelectedBackgroundColor(gf::Color::White);
    m_enterWidget.setBackgroundOutlineThickness(26 * .05f);
    m_enterWidget.setDefaultBackgroundOutlineColor(gf::Color::White);
    m_enterWidget.setSelectedBackgroundOutlineColor(gf::Color::White);
    m_enterWidget.setPadding(26 * .4f);
    target.draw(m_enterWidget);

    //target.display();
}
