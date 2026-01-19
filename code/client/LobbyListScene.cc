#include "LobbyListScene.h"

LobbyListScene::LobbyListScene(Renderer& renderer)
: m_entity(renderer)
{
}

LobbyListAction LobbyListScene::processEvent(const gf::Event& event){
    return m_entity.processEvent(event);
}

void LobbyListScene::render(){
    m_entity.render();
}
