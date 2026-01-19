#include "LobbyListEntity.h"
#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Log.h>
#include <gf/Anchor.h>
#include <gf/Coordinates.h>

LobbyListEntity::LobbyListEntity(Renderer& renderer)
: m_renderer(renderer)
, m_font("../common/fonts/arial.ttf")
, m_lastAction(LobbyListAction::None)
{
    
}

LobbyListAction LobbyListEntity::processEvent(const gf::Event& event) {
    gf::RenderWindow& win = m_renderer.getRenderWindow();

    switch (event.type) {
        case gf::EventType::MouseMoved: {
            
        }

        case gf::EventType::MouseButtonPressed: {
            if (event.mouseButton.button != gf::MouseButton::Left)
                return LobbyListAction::None;

            
        }

        default:
            return LobbyListAction::None;
    }
}
void LobbyListEntity::render() {

}
