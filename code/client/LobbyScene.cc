#include "LobbyScene.h"

LobbyScene::LobbyScene(Renderer& renderer)
: m_entity(renderer)
{
}

LobbyAction LobbyScene::processEvent(const gf::Event& event){
    return m_entity.processEvent(event);
}

void LobbyScene::render(std::vector<PlayerData> players,RoomSettings settings, int clientID){
    m_entity.render(players,settings,clientID);
}
