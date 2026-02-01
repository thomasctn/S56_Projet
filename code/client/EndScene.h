#pragma once

#include <gf/Scene.h>
#include <gf/Font.h>
#include "../common/Protocol.h" 

#include "EndEntity.h"

class ClientGame;

class EndScene : public gf::Scene {
public:
    explicit EndScene(ClientGame& game);

    //init end info (appelée par ClientGame quand on switch vers End)
    void initEnd(GameEndReason reason, int lastScore);

private:
    void doProcessEvent(gf::Event& event) override;
    void doUpdate(gf::Time time) override;

private:
    ClientGame& m_game;
    gf::Font m_font;
    EndEntity m_entity;
};
