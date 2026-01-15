#include "GameScene.h"
#include "Renderer.h"

GameScene::GameScene(Renderer& renderer)
: m_entity(renderer)
{
}

void GameScene::processEvent(const gf::Event& ) { //rien a faire pour l'isntant
}

void GameScene::render(const std::vector<PlayerData>& states,uint32_t myId,const BoardCommon& board,const std::set<Position>& pacgommes)
{
    m_entity.render(states, myId, board, pacgommes);
}
