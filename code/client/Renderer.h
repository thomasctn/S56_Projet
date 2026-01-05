#pragma once

#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Shapes.h>
#include <gf/Color.h>
#include <vector>
#include "Structures.h"

#include "../common/Types.h"

class Renderer{
    public:
    Renderer();
    void render(const std::vector<ClientState>& states, uint32_t myId, const mapRec map);
    void renderMap(const std::vector<ClientState>& states, uint32_t myId, const mapRec map);

    bool isOpen() {
        return main_window.isOpen();
    }
    gf::Window& getWindow(){
        return main_window;
    }

    private:
    gf::Window main_window;
    gf::RenderWindow rendered_window;
    gf::Color4f colorFromId(uint32_t id);

};