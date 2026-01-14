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

 
bool WelcomeEntity::processEvent(const gf::Event& event){
    switch (event.type) {
        case gf::EventType::MouseMoved: {
            //convertit les coordenéee de la fenetre en coord de mon monde
            auto winSize = m_renderer.getWindow().getSize();
            float wx = event.mouseCursor.coords.x * (m_renderer.getWorldSize() / float(winSize.x));
            float wy = event.mouseCursor.coords.y * (m_renderer.getWorldSize() / float(winSize.y));
            m_container.pointTo({wx, wy});
            return false;
        }

        case gf::EventType::MouseButtonPressed: {
            if (event.mouseButton.button != gf::MouseButton::Left) 
                return false;
            auto winSize = m_renderer.getWindow().getSize();
            float wx = event.mouseButton.coords.x * (m_renderer.getWorldSize() / float(winSize.x));
            float wy = event.mouseButton.coords.y * (m_renderer.getWorldSize() / float(winSize.y));
            m_container.pointTo({wx, wy});
            m_container.triggerAction();
            if (m_wasClicked) {
                m_wasClicked = false;
                return true;
            }
            return false;
        }

        default:
            return false;
    }
}




void WelcomeEntity::render() {
    m_renderer.clearWindow();
    gf::RenderWindow& target = m_renderer.getRenderWindow();

    float world = m_renderer.getWorldSize();

    //titre
    gf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(48);
    title.setColor(gf::Color::White);
    title.setString("PACMAN");
    title.setAnchor(gf::Anchor::TopCenter);
    title.setPosition({ world * 0.5f, world * 0.1f });
    target.draw(title);

    //bouton
    float bw = world * 0.4f;
    float bh = world * 0.12f;
    float bx = (world - bw) * 0.5f;
    float by = world * 0.45f;

    gf::RectangleShape button({ bw, bh });
    button.setPosition({ bx, by });
    button.setColor(gf::Color::fromRgb(50, 70, 200));
    target.draw(button);

    //texte du bouton
    m_enterWidget.setCharacterSize(26);
    m_enterWidget.setAnchor(gf::Anchor::Center);
    m_enterWidget.setPosition({ bx + bw * 0.5f, by + bh * 0.5f });

    target.draw(m_enterWidget);

    target.display();
}
