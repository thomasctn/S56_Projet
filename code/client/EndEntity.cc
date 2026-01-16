#include "EndEntity.h"
#include <gf/Coordinates.h>
#include <gf/RenderStates.h>

EndEntity::EndEntity(Renderer& renderer): m_renderer(renderer), m_font("../common/fonts/arial.ttf"), m_enterWidget("Retour au Lobby", m_font), m_wasClicked(false){
    m_enterWidget.setCallback([this](){
        m_wasClicked = true;
    });
    m_container.addWidget(m_enterWidget);
    m_enterWidget.setCharacterSize(24);
    m_enterWidget.setAnchor(gf::Anchor::Center);
}

bool EndEntity::wasClicked() const {
    return m_wasClicked;
}

void EndEntity::resetClick() {
    m_wasClicked = false;
}

 
bool EndEntity::processEvent(const gf::Event& event){
    gf::RenderWindow& win = m_renderer.getRenderWindow();

    switch (event.type) {

        case gf::EventType::MouseMoved: {
            gf::Vector2i pixelPos(int(event.mouseCursor.coords.x), int(event.mouseCursor.coords.y)); //a eventuellement changer pour le meme system que dans lobby!
            gf::Vector2f worldPos = win.mapPixelToCoords(pixelPos); // utilisation de la view actuelle
            m_container.pointTo(worldPos);
            return false;
        }

        case gf::EventType::MouseButtonPressed: {
            if (event.mouseButton.button != gf::MouseButton::Left) 
                return false;

            gf::Vector2i pixelPos(int(event.mouseButton.coords.x), int(event.mouseButton.coords.y));
            gf::Vector2f worldPos = win.mapPixelToCoords(pixelPos); 
            m_container.pointTo(worldPos);

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


void EndEntity::render(int lastScore, GameEndReason endReason) {
    m_renderer.clearWindow();
    gf::RenderWindow& target = m_renderer.getRenderWindow();

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
    title.setString("Fin de Jeu");
    title.setAnchor(gf::Anchor::TopCenter);
    title.setPosition({ left + width * 0.5f, top + height * 0.1f });
    target.draw(title);

    gf::Text reasonText;
    reasonText.setFont(m_font);
    reasonText.setCharacterSize(22);
    reasonText.setColor(gf::Color::White);
    reasonText.setAnchor(gf::Anchor::TopCenter);

    std::string reasonStr;
    switch (endReason) {
        case GameEndReason::ALL_DOT_EATEN:
            reasonStr = "Pacman a mangé toutes les pacgommes.\nPacman gagne !";
            break;
        case GameEndReason::TIME_OUT:
            reasonStr = "Le temps est écoulé.\nLes fantômes gagnent.";
            break;
        case GameEndReason::PACMAN_DEATH:
            reasonStr = "Pacman est mort trop de fois.\nLes fantômes gagnent.";
            break;
    }

    reasonText.setString(reasonStr);
    reasonText.setPosition({ left + width * 0.32f, top + height * 0.27f });
    target.draw(reasonText);

    gf::Text scoreText;
    scoreText.setFont(m_font);
    scoreText.setCharacterSize(20);
    scoreText.setColor(gf::Color::White);
    scoreText.setAnchor(gf::Anchor::TopCenter);
    scoreText.setString("Score final de Pacman : " + std::to_string(lastScore));
    scoreText.setPosition({ left + width * 0.32f, top + height * 0.37f });
    target.draw(scoreText);


    //bouton
    float bw = width * 0.4f;
    float bh = height * 0.12f;
    float bx = left + (width - bw) * 0.5f;
    float by = top+ height * 0.55f;

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

    target.display();
}
