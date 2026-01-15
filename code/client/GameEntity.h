#pragma once

#include <vector>
#include <set>
#include <gf/Texture.h>
#include <gf/Sprite.h>
#include <gf/AnimatedSprite.h>
#include <gf/Animation.h>
#include "../common/Protocol.h"

class Renderer;

class GameEntity {
public:
  explicit GameEntity(Renderer& renderer);

  // render du jeu (map + joueurs + pacgommes)
  void render(const std::vector<PlayerData>& states,uint32_t myId,const BoardCommon& board,const std::set<Position>& pacgommes);

private:
  //helpers pdnt le render du jeu
  void calculateMovement(const BoardCommon &map, float &tileSize, float &offsetX, float &offsetY);
  void renderMap(const BoardCommon &map, float tileSize, float offsetX, float offsetY);
  void renderPacGommes(const std::set<Position>& pacgommes, float tileSize, float offsetX, float offsetY);

private:
  Renderer& m_renderer;

  //Textures et sprites
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

  //état pacman local pour changer animation
  char m_pacmanDir = 'R';
  float m_lastPacmanX = 0.f;
  float m_lastPacmanY = 0.f;
  bool m_hasLastPacmanPos = false;
};
