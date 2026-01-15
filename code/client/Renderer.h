#pragma once

#include <gf/Window.h>
#include <gf/RenderWindow.h>
#include <gf/Shapes.h>
#include <gf/Color.h>
#include <vector>

#include <gf/Texture.h>
#include <gf/Sprite.h>
#include <gf/AnimatedSprite.h>
#include <gf/Animation.h>


#include "../common/Protocol.h"

class Renderer{
    public:
    Renderer();
    
   
    //modif pour que scenes fonctionnent
    void clearWindow();
    gf::RenderWindow& getRenderWindow();
    

    
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
    gf::View m_view; // la view que l'on manipule

    float m_logicalWidth = 500.f; // largeur logique totale
    float m_logicalHeight = 600.f; // hauteur logique totale (500 map + 100 pour texte)

    




};