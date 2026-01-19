#pragma once
#include <gf/Event.h>
#include "../common/Protocol.h"
#include "GameEntity.h"

class Renderer;

class GameScene {
public:
  explicit GameScene(Renderer& renderer);

  void processEvent(const gf::Event& event); //y a pas d'event pr l'instant mais bon

  // Render : passe au GameEntity
  void render(const std::vector<PlayerData>& states,uint32_t myId,const BoardCommon& board,const std::vector<std::pair<Position, PacGommeType>>& pacgommes, int timeLeftPre, unsigned int timeLeft);

private:
  GameEntity m_entity;
};
