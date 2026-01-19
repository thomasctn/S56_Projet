#pragma once
#include <gf/Event.h>
#include <gf/WidgetContainer.h>
#include <gf/Widgets.h>
#include <gf/Text.h>
#include <gf/Font.h>
#include "Renderer.h"
#include "../common/Types.h"

enum class LobbyListAction {// actions possibles renvoyées par l'entity au main/scene

    None
};

class LobbyListEntity {
public:
    explicit LobbyListEntity(Renderer& renderer);

    LobbyListAction processEvent(const gf::Event& event);

    void render();

private:
    Renderer& m_renderer;
    gf::Font m_font;

    LobbyListAction m_lastAction;
};
