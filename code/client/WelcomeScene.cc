#include "WelcomeScene.h"

WelcomeScene::WelcomeScene(Renderer& renderer): m_entity(renderer){
}

bool WelcomeScene::processEvent(const gf::Event& event) {
    return m_entity.processEvent(event);
}


void WelcomeScene::render() {
    m_entity.render();
}
