#pragma once
#include <gf/Scene.h>
#include <chrono>
#include "../common/Protocol.h"
#include "GameEntity.h"

class ClientGame;

class GameScene : public gf::Scene {
public:
    explicit GameScene(ClientGame& game);

  void setInitialState(const std::vector<PlayerData>& players,const BoardCommon& board,const std::map<Position, Position>& holeLinks);


private:
    void doProcessEvent(gf::Event& event) override;
    void doUpdate(gf::Time time) override;

private:
    ClientGame& m_game;
    GameEntity m_entity;
    std::chrono::steady_clock::time_point m_lastSend;
};