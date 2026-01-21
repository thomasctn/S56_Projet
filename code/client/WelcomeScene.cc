#include "WelcomeScene.h"

WelcomeScene::WelcomeScene(Renderer& renderer): m_renderer(renderer), m_entity(renderer){
}

bool WelcomeScene::processEvent(const gf::Event& event) {
    gf::RenderWindow& win = m_renderer.getRenderWindow();

    switch (event.type) {

        case gf::EventType::MouseMoved: {
            gf::Vector2i pixelPos(
                int(event.mouseCursor.coords.x),
                int(event.mouseCursor.coords.y)
            );
            gf::Vector2f worldPos = win.mapPixelToCoords(pixelPos);
            m_entity.pointTo(worldPos);
            return false;
        }

        case gf::EventType::MouseButtonPressed: {
            if (event.mouseButton.button != gf::MouseButton::Left)
                return false;

            gf::Vector2i pixelPos(
                int(event.mouseButton.coords.x),
                int(event.mouseButton.coords.y)
            );
            gf::Vector2f worldPos = win.mapPixelToCoords(pixelPos);
            m_entity.pointTo(worldPos);

            return m_entity.trigger();
        }

        default:
            return false;
    }
}



void WelcomeScene::render() {
    gf::RenderWindow& win = m_renderer.getRenderWindow();
    m_renderer.clearWindow();
    m_entity.render(win);
    win.display();
}

