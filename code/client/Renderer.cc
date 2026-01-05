#include "Renderer.h"

#include "Structures.h"
#include <gf/Log.h>

Renderer::Renderer() : main_window("GF Sync Boxes", {800,600}), rendered_window(main_window){
    m_view.setSize({m_worldSize, m_worldSize});
    m_view.setCenter({m_worldSize/ 2.f, m_worldSize / 2.f});
    rendered_window.setView(m_view);
}

gf::Color4f Renderer::colorFromId(uint32_t id) {
    float r = float((id * 50) % 256) / 255.0f;
    float g = float((id * 80) % 256) / 255.0f;
    float b = float((id * 110) % 256) / 255.0f;
    return gf::Color4f(r, g, b, 1.0f);
}

void Renderer::render(const std::vector<ClientState>& states, uint32_t myId, const mapRec map){
    rendered_window.clear(gf::Color::Black);

    for (auto& s : states) {
        gf::RectangleShape box({50.0f, 50.0f});
        box.setPosition({s.x, s.y});
        gf::Color4f c = (s.id == myId) ? gf::Color4f(1, 0, 0, 1) : colorFromId(s.id);
        box.setColor(c);
        rendered_window.draw(box);
    }
/* VIELLE MAP PERSO
    int map_size = map.size();
    for (int y = 0; y < map_size; y++) {
        for (int x = 0; x < map_size; x++) {
            int tile = map[y][x];
            if (tile == -1) continue;

            gf::RectangleShape tileRect({50.0f, 50.0f});
            tileRect.setPosition({float(x * 50), float(y * 50)});

            if (tile == 0) tileRect.setColor(gf::Color4f(0.5f, 0.5f, 0.5f, 1.0f));
            if (tile == 1) tileRect.setColor(gf::Color::White);
            if (tile == 2) tileRect.setColor(gf::Color::Red);

            rendered_window.draw(tileRect);
        }
    }*/
    renderMap(states,myId,map);
    rendered_window.display();


}



void Renderer::renderMap(const std::vector<ClientState>& states, uint32_t myId, const mapRec map){
    mapRec mapPerso = map;
    //sans le responsive:
    const float RENDER_SIZE = 500.0f;
    float tileSize = std::min(RENDER_SIZE / mapPerso.width, RENDER_SIZE / mapPerso.height);
    float offsetX = (m_worldSize - tileSize * mapPerso.width) / 2.f;
    float offsetY = (m_worldSize - tileSize * mapPerso.height) / 2.f;



    for (int y = 0; y < mapPerso.height; ++y) {
                for (int x = 0; x < mapPerso.width; ++x) {
                    const CaseRec& cell = mapPerso.grid[y][x];

                    gf::RectangleShape tile({tileSize, tileSize});
                    tile.setPosition({x * tileSize + offsetX, y * tileSize + offsetY});

                    switch (cell.type) {
                        case CellType::Wall: tile.setColor(gf::Color::White); break;
                        case CellType::Hut:  tile.setColor(gf::Color::Red); break;
                        case CellType::Floor:
                        default: continue;
                    }
                    rendered_window.draw(tile);
                }
            }
}

void Renderer::handleResize(unsigned int winW, unsigned int winH)
{
    float windowRatio = float(winW) / float(winH);
    float worldRatio  = 1.0f; // monde carré

    if (windowRatio > worldRatio) {
        m_view.setSize({m_worldSize* windowRatio, m_worldSize}); // fenêtre trop large
    } else {
        m_view.setSize({m_worldSize, m_worldSize/windowRatio}); // fenêtre trop haute
    }

    m_view.setCenter({m_worldSize / 2.f, m_worldSize / 2.f});
    rendered_window.setView(m_view);
}

