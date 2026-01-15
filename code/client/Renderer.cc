#include "Renderer.h"

#include <gf/Log.h>
#include <gf/Text.h>
#include <gf/Font.h>

Renderer::Renderer() : main_window("GF Sync Boxes", {800,600}), rendered_window(main_window){  
    
    m_view.setSize({ m_logicalWidth, m_logicalHeight });
    m_view.setCenter({ m_logicalWidth / 2.f, m_logicalHeight / 2.f });
    rendered_window.setView(m_view);

}



//modif pour que les scenes fonctionnent

void Renderer::clearWindow(){
    rendered_window.clear(gf::Color::Black);
}

gf::RenderWindow& Renderer::getRenderWindow(){
    return rendered_window;
}


void Renderer::handleResize(unsigned int winW, unsigned int winH)
{
    float windowRatio = float(winW) / float(winH);
    float logicalRatio = m_logicalWidth / m_logicalHeight;

    if (windowRatio > logicalRatio) {
        m_view.setSize({m_logicalHeight * windowRatio, m_logicalHeight}); // fenêtre trop large
    } else {
        m_view.setSize({m_logicalWidth, m_logicalWidth / windowRatio}); // fenêtre trop haute
    }

    m_view.setCenter({m_logicalWidth / 2.f, m_logicalHeight / 2.f});
    rendered_window.setView(m_view);
}


