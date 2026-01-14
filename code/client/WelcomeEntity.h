#pragma once
#include <gf/Entity.h>
#include <gf/WidgetContainer.h>
#include <gf/Widgets.h>
#include <gf/Text.h>
#include <gf/Font.h>
#include <gf/Event.h>
#include "Renderer.h"

class WelcomeEntity {
public:
    WelcomeEntity(Renderer& renderer);
    bool processEvent(const gf::Event& event);
    void render();
    bool wasClicked() const;
    void resetClick();


private:
    Renderer& m_renderer;
    gf::Font m_font;
    gf::WidgetContainer m_container;
    gf::TextWidget m_enterWidget;
    bool m_wasClicked;
    
};
