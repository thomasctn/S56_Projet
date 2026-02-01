#include "EndScene.h"
#include "ClientGame.h"
#include <gf/Log.h>

EndScene::EndScene(ClientGame& game)
: gf::Scene(gf::vec(1280, 720))
, m_game(game)
, m_font("../common/fonts/arial.ttf")
, m_entity()
{
    setClearColor(gf::Color::Black);
    addHudEntity(m_entity);
}

void EndScene::initEnd(GameEndReason reason, int lastScore){ //appelé par le clientgame
    m_entity.setEndData(reason, lastScore);
}

void EndScene::doProcessEvent(gf::Event& event) {
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

void EndScene::doUpdate(gf::Time){
    EndAction act = m_entity.getAndResetLastAction();
    if (act == EndAction::BackToLobby) {
        m_game.requestScene(SceneRequest::GoToLobby);
    }
}
