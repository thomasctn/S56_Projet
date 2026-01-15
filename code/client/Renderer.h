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
    void render(const std::vector<PlayerData>& states, uint32_t myId, const BoardCommon map,const std::set<Position>& pacgommes);
    void renderMap(const std::vector<PlayerData>& states,  const BoardCommon map);
    void calculateMovement( const BoardCommon& map,float& tileSize, float& offsetX, float& offsetY);
    void renderPacGommes(const std::set<Position>& pacgommes,float tileSize,float offsetX,float offsetY);

    void renderWelcome();
    void renderLobby(int connectedPlayers,int roomSize,bool amReady, int nbBots, int gameDur, PlayerRole myRole);

    //modif pour que scenes fonctionnent
    void clearWindow();
    gf::RenderWindow& getRenderWindow();
    

    
    bool isOpen() {
        return main_window.isOpen();
    }
    gf::Window& getWindow(){
        return main_window;
    }

   // float getWorldSize() const { return m_worldSize; }

    gf::v1::Vector2f getMinusBtnPos() const { return m_minusBtnPos; }
    gf::v1::Vector2f getPlusBtnPos() const { return m_plusBtnPos; }
    gf::v1::Vector2f getMinusBotBtnPos() const { return m_minusBotBtnPos; }
    gf::v1::Vector2f getPlusBotBtnPos() const { return m_plusBotBtnPos; }
    gf::v1::Vector2f getMinusDurBtnPos() const { return m_minusDurBtnPos; }
    gf::v1::Vector2f getPlusDurBtnPos() const { return m_plusDurBtnPos; }
    gf::v1::Vector2f getReadyBtnPos() const { return m_readyBtnPos; }

    gf::v1::Vector2f getChangeRoleBtnPos() const { return m_CRBtnPos; }

    gf::v1::Vector2f getBtnSize() const { return m_btnSize; }           // - et +
    gf::v1::Vector2f getReadyBtnSize() const { return m_readyBtnSize; } // PRET

    void handleResize(unsigned int winW, unsigned int winH);

    private:
    gf::Window main_window;
    gf::RenderWindow rendered_window;
    gf::Color4f colorFromId(uint32_t id);
    gf::View m_view; // la view que l'on manipule
   // float m_worldSize = 500.f;  //taille de la map basique

    float m_logicalWidth = 500.f; // largeur logique totale
    float m_logicalHeight = 600.f; // hauteur logique totale (500 map + 100 pour texte)

    gf::Texture m_inkyTexture;
    gf::Sprite  m_inkySprite;
    gf::Texture m_clydeTexture;
    gf::Sprite  m_clydeSprite;
    gf::Texture m_pinkyTexture;
    gf::Sprite  m_pinkySprite;
    gf::Texture m_blinkyTexture;
    gf::Sprite  m_blinkySprite;

    //gf::Texture m_pacmanTexture;
    //gf::Animation m_pacmanAnimation;
    gf::AnimatedSprite m_pacmanSprite;   

    //pour les differentes directions animée
    char m_pacmanDir = 'R'; //R L U D
    float m_lastPacmanX = 0.f;
    float m_lastPacmanY = 0.f;
    bool m_hasLastPacmanPos = false;

    gf::Texture m_pacmanRightTexture;
    gf::Texture m_pacmanLeftTexture;
    gf::Texture m_pacmanUpTexture;
    gf::Texture m_pacmanDownTexture;

    gf::Animation m_pacmanRightAnim;
    gf::Animation m_pacmanLeftAnim;
    gf::Animation m_pacmanUpAnim;
    gf::Animation m_pacmanDownAnim;

    //problems de clics
    // coordonnées et tailles dans le monde
    gf::v1::Vector2f m_minusBtnPos;
    gf::v1::Vector2f m_plusBtnPos;
    gf::v1::Vector2f m_readyBtnPos;

    gf::v1::Vector2f m_minusBotBtnPos;
    gf::v1::Vector2f m_plusBotBtnPos;

    gf::v1::Vector2f m_minusDurBtnPos;
    gf::v1::Vector2f m_plusDurBtnPos;


    gf::v1::Vector2f m_CRBtnPos;

    gf::v1::Vector2f m_btnSize;       // pour - et +
    gf::v1::Vector2f m_readyBtnSize;  // pour le bouton PRET    




};