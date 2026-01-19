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

void LobbyListScene::setRooms(const std::vector<RoomData>& rooms) {
    m_entity.setRooms(rooms);
}

LobbyListAction LobbyListScene::getAndResetLastAction() {
    return m_entity.getAndResetLastAction();
}

unsigned int LobbyListScene::getLastRoomId() const {
    return m_entity.getLastRoomId();
}
