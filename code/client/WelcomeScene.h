#pragma once
#include <gf/Event.h>
#include "WelcomeEntity.h"

class Renderer;

class WelcomeScene {
public:
    explicit WelcomeScene(Renderer& renderer);

    bool processEvent(const gf::Event& event); //true quand on clique sur enetrer
    void render();

private:
    WelcomeEntity m_entity;
    Renderer& m_renderer;
};
