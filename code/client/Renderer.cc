#include "Renderer.h"

#include <gf/Log.h>

Renderer::Renderer() : main_window("GF Sync Boxes", {800,600}), rendered_window(main_window){
    //POUR LE SPRITE 
    m_inkyTexture = gf::Texture("../client/assets/ghosts/inky.png"); 
    m_inkySprite.setTexture(m_inkyTexture);
    m_inkySprite.setOrigin({0.f, 0.f}); //en haut a gauche
    m_inkySprite.setScale({2.f, 2.f}); //sa taille (temp ce sera recalculé norùalement)
    
    m_clydeTexture = gf::Texture("../client/assets/ghosts/clyde.png"); 
    m_clydeSprite.setTexture(m_clydeTexture);
    m_clydeSprite.setOrigin({0.f, 0.f}); //en haut a gauche
    m_clydeSprite.setScale({2.f, 2.f}); //sa taille (temp ce sera recalculé norùalement)

    //tests pacman animé
    m_pacmanTexture = gf::Texture("../client/assets/pacman/right_pacman_sheet.png");
    m_pacmanAnimation.addTileset(m_pacmanTexture, {4,1}, gf::seconds(0.1f), 4); 
    m_pacmanAnimation.setLoop(true);

    // Lier l'animation au sprite
    m_pacmanSprite.setAnimation(m_pacmanAnimation);

    // Centrer ou placer l'origine si nécessaire
    m_pacmanSprite.setOrigin({0.f, 0.f});

    
    
    
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




void Renderer::render(const std::vector<ClientState>& states, uint32_t myId, const BoardCommon map){
    rendered_window.clear(gf::Color::Black);
    
    //AVANT : LES PXIELS JOUEURS
    /*for (auto& s : states) {
        gf::RectangleShape box({50.0f, 50.0f});
        box.setPosition({s.x, s.y});
        gf::Color4f c = (s.id == myId) ? gf::Color4f(1, 0, 0, 1) : colorFromId(s.id);
        box.setColor(c);
        rendered_window.draw(box);
    }*/

    //apres, test sprit:
    float tileSize, offsetX, offsetY;
    calculateMovement(m_worldSize, map, tileSize, offsetX, offsetY);

    //TEMPORAIRE calculer correctement la taille du sprite!
    auto texSize=m_inkyTexture.getSize();
    if (texSize.x>0 && texSize.y>0){
        float scaleX= tileSize/float(texSize.x);
        float scaleY= tileSize/float(texSize.y);
        m_inkySprite.setScale({scaleX, scaleY });
        m_clydeSprite.setScale({scaleX, scaleY });
    }

    for (const auto &s : states) {
        float px = s.x / 50.0f * tileSize + offsetX;
        float py = s.y / 50.0f * tileSize + offsetY;
        if(s.id == 1){
            m_pacmanSprite.setPosition({ px, py });

            auto texSize = m_pacmanTexture.getSize();
            float scaleX = tileSize / float(texSize.x / 4); //divisé par 4 frames
            float scaleY = tileSize / float(texSize.y);     
            m_pacmanSprite.setScale({scaleX, scaleY});

            m_pacmanSprite.update(gf::seconds(0.008f));  //maj de l'anim
            rendered_window.draw(m_pacmanSprite);
        }
        else{
            m_inkySprite.setPosition({ px, py });
            rendered_window.draw(m_inkySprite);
        }
    }

    /*Pour clyde
    m_clydeSprite.setPosition({ px, py });
            rendered_window.draw(m_clydeSprite);*/


    /*for (const auto& s : states) {
        m_inkySprite.setPosition({s.x, s.y});
        rendered_window.draw(m_inkySprite);
    }*/

  
    renderMap(states,myId,map);
    rendered_window.display();


}



void Renderer::renderMap(const std::vector<ClientState>& states, uint32_t myId, const BoardCommon map){
    BoardCommon mapPerso = map;
    //sans le responsive:
    float tileSize = std::min(m_worldSize / mapPerso.width, m_worldSize / mapPerso.height); //a remplacer par ma fonction utilistaire
    float offsetX = (m_worldSize - tileSize * mapPerso.width) / 2.f;
    float offsetY = (m_worldSize - tileSize * mapPerso.height) / 2.f;



    for (unsigned int y = 0; y < mapPerso.height; ++y) {
                for (unsigned int x = 0; x < mapPerso.width; ++x) {
                    const CaseCommon& cell = mapPerso.grid({ x, y });

                    gf::RectangleShape tile({tileSize, tileSize});
                    tile.setPosition({x * tileSize + offsetX, y * tileSize + offsetY});

                    switch (cell.celltype) {
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


void Renderer::calculateMovement(float worldSize, const BoardCommon &map, float &tileSize, float &offsetX, float &offsetY) {
    tileSize = std::min(worldSize / float(map.width), worldSize / float(map.height));

    offsetX = (worldSize - tileSize *float(map.width)) / 2.f;
    offsetY = (worldSize - tileSize *float(map.height)) / 2.f;
}