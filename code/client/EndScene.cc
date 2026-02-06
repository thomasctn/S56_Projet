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
    addWorldEntity(m_entity);
}

void EndScene::initEnd(GameEndReason reason, int lastScore){ //appelé par le clientgame
    m_entity.setEndData(reason, lastScore);
}

void EndScene::doProcessEvent(gf::Event& event) {
    switch (event.type) {
        case gf::EventType::MouseMoved:
            m_entity.pointTo(
                m_game.computeWindowToGameCoordinates(event.mouseCursor.coords, getWorldView())
            );
            break;

        case gf::EventType::MouseButtonPressed:
            m_entity.pointTo(
                m_game.computeWindowToGameCoordinates(event.mouseButton.coords, getWorldView())
            );
            m_entity.triggerAction();
            break;

        case gf::EventType::Closed: 
            gf::Log::info("Fenêtre fermée (LobbyListScene)\n");
            m_game.shutdown();
            break;
        case gf::EventType::Resized: {
            resizeYourself();
            break;
        }

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

void EndScene::resizeYourself(){
    auto size = m_game.getWindow().getSize(); 
    m_game.handleResize(size.x, size.y);
    getWorldView() = m_game.getMainView();
}

