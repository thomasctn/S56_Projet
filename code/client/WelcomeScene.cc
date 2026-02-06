
#include "WelcomeScene.h"
#include "ClientGame.h"


WelcomeScene::WelcomeScene(ClientGame& game)
: gf::Scene(gf::vec(1280,720))  
, m_game(game)
, m_font("../common/fonts/arial.ttf")
, m_entity(m_font)
{
    setClearColor(gf::Color::Black);
    addWorldEntity(m_entity);
    
}

void WelcomeScene::doProcessEvent(gf::Event& event) {
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

void WelcomeScene::doUpdate(gf::Time time) {
    gf::Packet packet;
    while (m_game.tryPopPacket(packet)) {
        switch (packet.getType()) {
            case ServerConnect::type:{
                auto data = packet.as<ServerConnect>();
                m_game.setMyId(data.clientId);
                gf::Log::info("ID client assigné par le serveur : %u\n", m_game.getMyId());
                break;
            }

            case ServerListRooms::type:{
                auto data = packet.as<ServerListRooms>();
                m_game.setLastRooms(data.rooms);
                gf::Log::info("serverlistrooms reçu dans WelcomeScene : %zu\n", data.rooms.size());
                break;
            }

            default:
                break;
        }
    }

    //logique du bouton
    if (m_entity.wasClicked()) {
        m_entity.resetClick();
        //faut envoyer en reseau notre requete et changer la scene...
        //enft non ici y a pas besoin de reseau, juste demander le schangement de scene et donc recup et affichage lobbylist
        //mais pr dautres scene ici y aura surement besoin de dire a scenemanager/gamemamaner d'envoyer des trucs!!!
        m_game.requestScene(SceneRequest::GoToLobbyList);
    }
}

void WelcomeScene::resizeYourself(){
    auto size = m_game.getWindow().getSize(); 
    m_game.handleResize(size.x, size.y);
    getWorldView() = m_game.getMainView();
}
