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

    void handleResize(unsigned int winW, unsigned int winH);

    private:
    gf::Window main_window;
    gf::RenderWindow rendered_window;
    gf::Color4f colorFromId(uint32_t id);
    gf::View m_view;            // la view que l'on manipule
    float m_baseWidth = 800.0f; // taille logique initiale (comme la fenêtre)
    float m_baseHeight = 600.0f;
    float m_worldSize = 500.f;  //taille de la map basique


};