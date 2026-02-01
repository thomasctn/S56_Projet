#include "GameScene.h"
#include "ClientGame.h"
#include <gf/Log.h>

GameScene::GameScene(ClientGame& game)
: gf::Scene(gf::vec(1280, 720))
, m_game(game)
, m_entity()
, m_lastSend(std::chrono::steady_clock::now())
{
    setClearColor(gf::Color::Black);
    addHudEntity(m_entity);
}

void GameScene::setInitialState(
    const std::vector<PlayerData>& players,
    const BoardCommon& board,
    const std::map<Position, Position>& holeLinks
) {
    m_entity.setGameState(players);
    m_entity.setBoard(board);
    m_entity.setHoleLinks(holeLinks);
    m_entity.setClientId(m_game.getMyId());
}

void GameScene::doProcessEvent(gf::Event& event) {
    //gf::Log::info(" do process event\n");

    m_game.getActions().processEvent(event);
}

void GameScene::doUpdate(gf::Time time) {
    //gf::Log::info("gamescene DO UPDATE\n");

    //paquets reseau
    gf::Packet packet;
    while (m_game.tryPopPacket(packet)) {
        switch (packet.getType()) {
            case ServerGamePreStart::type: {
                auto data = packet.as<ServerGamePreStart>();
                m_entity.setTimeLeft(int(data.timeLeft), 0);
                //gf::Log::info("servergameprestart, timeleft recu: %d\n", static_cast<int>(data.timeLeft));

                break;
            }



            
            case ServerGameState::type: {
                auto data = packet.as<ServerGameState>();
                m_entity.setGameState(data.clientStates);
                
                std::vector<std::pair<Position, PacGommeType>> pacgommes;
                for (auto& [pos, type] : data.pacgommes) {
                    pacgommes.push_back({pos, type});
                }
                m_entity.setPacGommes(pacgommes);
                m_entity.setGameTimeLeft(data.timeLeft);
                //gf::Log::info("servergamestate, timeleft recu: %d\n", static_cast<int>(data.timeLeft));

                break;
            }
            
            case ServerGameEnd::type: {
                auto data = packet.as<ServerGameEnd>();
                gf::Log::info("Game end reçu, raison: %d\n", static_cast<int>(data.reason));
                m_game.goToEndScene(data.reason);
                break;
            }
            
            default:
                break;
        }
    }
    
    //envoi de la tocueh seulement si clock a commencé
    auto now = std::chrono::steady_clock::now();
    if (now - m_lastSend > std::chrono::milliseconds(1000 / CLIENT_MOVE_SPEED)) {
        char dir = 0;
        
        if (m_game.upAction.isActive()) {
            dir = 'U';
        } else if (m_game.downAction.isActive()) {
            dir = 'D';
        } else if (m_game.leftAction.isActive()) {
            dir = 'L';
        } else if (m_game.rightAction.isActive()) {
            dir = 'R';
        }
        
        if (dir != 0) {
            //gf::Log::info("Envoi : touche '%c'\n", dir);
            gf::Packet pkt;
            ClientMove cm;
            cm.moveDir = dir;
            pkt.is(cm);
            m_game.getSocket().sendPacket(pkt);
            m_lastSend = now;
        }
    }
}