#include "LobbyScene.h"

LobbyScene::LobbyScene(Renderer& renderer)
: m_entity(renderer)
{
}

LobbyAction LobbyScene::processEvent(const gf::Event& event){
    return m_entity.processEvent(event);
}

void LobbyScene::render(int connectedPlayers,int roomSize,bool amReady,int nbBots,int gameDur, PlayerRole myRole){
    m_entity.render(connectedPlayers, roomSize, amReady, nbBots, gameDur, myRole);
}
