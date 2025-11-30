#include "Renderer.h"
#include <gf/Log.h>

Renderer::Renderer() : main_window("GF Sync Boxes", {800,600}), rendered_window(main_window){}

gf::Color4f Renderer::colorFromId(uint32_t id) {
    float r = float((id * 50) % 256) / 255.0f;
    float g = float((id * 80) % 256) / 255.0f;
    float b = float((id * 110) % 256) / 255.0f;
    return gf::Color4f(r, g, b, 1.0f);
}

void Renderer::render(const std::vector<ClientState>& states, uint32_t myId, const std::vector<std::vector<int>>& map){
    rendered_window.clear(gf::Color::Black);

    for (auto& s : states) {
        gf::RectangleShape box({50.0f, 50.0f});
        box.setPosition({s.x, s.y});
        gf::Color4f c = (s.id == myId) ? gf::Color4f(1, 0, 0, 1) : colorFromId(s.id);
        box.setColor(c);
        rendered_window.draw(box);
    }

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
    }

    rendered_window.display();


}