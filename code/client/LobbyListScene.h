#pragma once
#include <gf/Event.h>
#include "LobbyListEntity.h"
#include "Renderer.h"

class LobbyListScene {
public:
    explicit LobbyListScene(Renderer& renderer);

    LobbyListAction processEvent(const gf::Event& event);

    void render();

private:
    LobbyListEntity m_entity;
};
