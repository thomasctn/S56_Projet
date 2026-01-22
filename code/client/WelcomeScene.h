#pragma once

#include <gf/Scene.h>
#include <gf/Font.h>

#include "WelcomeEntity.h"
#include <gf/Scene.h>

class ClientGame;

class WelcomeScene : public gf::Scene {
public:
    explicit WelcomeScene(ClientGame& game);

private:
    void doProcessEvent(gf::Event& event) override;
    void doUpdate(gf::Time time) override;

private:
    ClientGame& m_game;
    gf::Font m_font;
    WelcomeEntity m_entity;
};
