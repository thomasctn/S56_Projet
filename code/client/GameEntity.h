#pragma once
#include <vector>
#include <map>
#include <gf/Entity.h>
#include <gf/Font.h>
#include <gf/Texture.h>
#include <gf/Sprite.h>
#include <gf/AnimatedSprite.h>
#include <gf/Animation.h>
#include "../common/Protocol.h"
#include "../common/Constants.h"

class GameEntity : public gf::Entity {
public:
    GameEntity();
    
    void setGameState(const std::vector<PlayerData>& states);
    void setBoard(const BoardCommon& board);
    void setPacGommes(const std::vector<std::pair<Position, PacGommeType>>& pacgommes);
    void setTimeLeft(int timeLeftPre, unsigned int timeLeft);
    void setGameTimeLeft(unsigned int timeLeft); //ne touche que timeLeft, pas timeLeftPre
    void setHoleLinks(const std::map<Position, Position>& holeLinks);
    void setClientId(uint32_t id);
    bool posIsInRange(int oX,int oY,int dX,int dY,double range) {
        return sqrt(pow(abs(oX-dX),2) + pow(abs(oY-dY),2)) <= range;
    }
    void update(gf::Time time) override;
    
void render(gf::RenderTarget& target, const gf::RenderStates& states) override;

private:
    void calculateMovement(gf::RenderTarget& target, const BoardCommon &map, float &tileSize, float &offsetX, float &offsetY);
    void renderMap(gf::RenderTarget& target, const gf::RenderStates& states, const BoardCommon& map, float logicalTileSize, float mapOriginX, float mapOriginY);
    void renderPacGommes(gf::RenderTarget& target, const gf::RenderStates& states, const std::vector<std::pair<Position, PacGommeType>>& pacgommes, float logicalTileSize, float mapOriginX, float mapOriginY);

private:
    //etat du jeu
    std::vector<PlayerData> m_states;
    BoardCommon m_board;
    std::vector<std::pair<Position, PacGommeType>> m_pacgommes;
    int m_timeLeftPre = 0;
    unsigned int m_timeLeft = 0;
    std::map<Position, Position> m_holeLinks;
    uint32_t m_clientId = 0;
    
    gf::Font m_font;
    
    //txtures et sprites
    gf::Texture m_inkyTexture;
    gf::Sprite  m_inkySprite;
    gf::Texture m_clydeTexture;
    gf::Sprite  m_clydeSprite;
    gf::Texture m_pinkyTexture;
    gf::Sprite  m_pinkySprite;
    gf::Texture m_blinkyTexture;
    gf::Sprite  m_blinkySprite;
    
    gf::AnimatedSprite m_pacmanSprite;
    gf::Texture m_pacmanRightTexture;
    gf::Texture m_pacmanLeftTexture;
    gf::Texture m_pacmanUpTexture;
    gf::Texture m_pacmanDownTexture;
    gf::Animation m_pacmanRightAnim;
    gf::Animation m_pacmanLeftAnim;
    gf::Animation m_pacmanUpAnim;
    gf::Animation m_pacmanDownAnim;
    
    //etat pacman local pour changer animation
    char m_pacmanDir = 'R';
    float m_lastPacmanX = 0.f;
    float m_lastPacmanY = 0.f;
    bool m_hasLastPacmanPos = false;
};