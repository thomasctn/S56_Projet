
#include "WelcomeScene.h"
#include "ClientGame.h"


WelcomeScene::WelcomeScene(ClientGame& game)
: gf::Scene(gf::vec(1280,720))  
, m_game(game)
, m_font("../common/fonts/arial.ttf")
, m_entity(m_font)
{
  addHudEntity(m_entity);
}

void WelcomeScene::doProcessEvent(gf::Event& event) {
    switch (event.type) {
        case gf::EventType::MouseMoved:
        m_entity.pointTo(
            m_game.computeWindowToGameCoordinates(event.mouseCursor.coords, getHudView())
        );
        break;

        case gf::EventType::MouseButtonPressed:
        m_entity.pointTo(
            m_game.computeWindowToGameCoordinates(event.mouseButton.coords, getHudView())
        );
        m_entity.triggerAction();
        break;

        default:
        break;
    }
}

void WelcomeScene::doUpdate(gf::Time time) {
    if (m_entity.wasClicked()) {
        m_entity.resetClick();
        //ici faut passer a la scene suivante?
    }
}
