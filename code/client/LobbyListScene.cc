#include "LobbyListScene.h"
#include "ClientGame.h"
#include <gf/Log.h>

LobbyListScene::LobbyListScene(ClientGame& game)
: gf::Scene(gf::vec(1280, 720))
, m_game(game)
, m_entity()
{
    setClearColor(gf::Color::Black);
    addWorldEntity(m_entity);

}

void LobbyListScene::doProcessEvent(gf::Event& event) {
    switch (event.type) {
        case gf::EventType::MouseMoved:
            m_entity.pointTo(
                m_game.computeWindowToGameCoordinates(event.mouseCursor.coords, getWorldView())
            );
            break;

        case gf::EventType::MouseButtonPressed:
            m_entity.pointTo(
                m_game.computeWindowToGameCoordinates(event.mouseButton.coords, getWorldView())
            );
            m_entity.triggerAction();
            break;

        case gf::EventType::Closed: 
            gf::Log::info("Fenêtre fermée (LobbyListScene)\n");
            m_game.shutdown();
            break;

        case gf::EventType::Resized: {
            resizeYourself();
            break;
        }

        default:
            break;
    }
}


void LobbyListScene::doUpdate(gf::Time time) {
    (void) time;

    gf::Packet packet;
    while (m_game.tryPopPacket(packet)) {
        switch (packet.getType()) {
            case ServerListRooms::type: {
                auto data = packet.as<ServerListRooms>();
                m_entity.setRooms(data.rooms);
                gf::Log::info("serverlistroomsrecu et setrooms lancé : %u\n", m_game.getMyId());

                break;
            }

            case ServerJoinRoom::type: {
                        gf::Log::info("Serveur: rejoint la room\n"); 
                        m_game.requestScene(SceneRequest::GoToLobby);

                        
                        break;
                    }

            case ServerConnect::type: {
                auto data = packet.as<ServerConnect>();
                m_game.setMyId(data.clientId);
                gf::Log::info("ID client assigné par le serveur : %u\n", m_game.getMyId());
                break;
            }

            default:
                break;
        }
    }

    LobbyListAction act = m_entity.getAndResetLastAction();
    if (act == LobbyListAction::CreateRoom) {
        gf::Packet p;
        ClientCreateRoom create;
        p.is(create);
        m_game.getSocket().sendPacket(p);
        gf::Log::info("ClientCreateRoom envoyé\n");
    } else if (act == LobbyListAction::JoinRoom) {
        unsigned int roomId = m_entity.getLastRoomId();
        gf::Packet p;
        ClientJoinRoom join;
        join.room = gf::Id(roomId);
        p.is(join);
        m_game.getSocket().sendPacket(p);
        gf::Log::info("ClientJoinRoom envoyé pour room %u\n", roomId);
    }


}

void LobbyListScene::resizeYourself(){
    auto size = m_game.getWindow().getSize(); 
    m_game.handleResize(size.x, size.y);
    getWorldView() = m_game.getMainView();
}