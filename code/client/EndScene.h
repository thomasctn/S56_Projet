#pragma once
#include <gf/Event.h>
#include "EndEntity.h"

class Renderer;

class EndScene {
public:
    explicit EndScene(Renderer& renderer);

    bool processEvent(const gf::Event& event); //true quand on clique sur enetrer
    void render();

private:
    EndEntity m_entity;
};
