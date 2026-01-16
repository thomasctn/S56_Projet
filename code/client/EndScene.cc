#include "EndScene.h"

EndScene::EndScene(Renderer& renderer): m_entity(renderer){
}

bool EndScene::processEvent(const gf::Event& event) {
    return m_entity.processEvent(event);
}


void EndScene::render(int lastScore, GameEndReason endReason) {
    m_entity.render(lastScore, endReason);
}
