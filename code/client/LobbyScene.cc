#include "LobbyScene.h"
#include "ClientGame.h"

#include <gf/Log.h>

LobbyScene::LobbyScene(ClientGame& game)
: gf::Scene(gf::vec(1280, 720))
, m_game(game)
, m_font("../common/fonts/arial.ttf")
, m_entity()
{
    setClearColor(gf::Color::Black);
    addWorldEntity(m_entity);
}

void LobbyScene::setInitialState(
    const std::vector<PlayerData>& players,
    const RoomSettings& settings
) {
    m_players = players;
    m_roomSettings = settings;

    m_entity.setPlayers(m_players);
    m_entity.setRoomSettings(m_roomSettings);
    m_entity.setClientId(m_game.getMyId());
}

void LobbyScene::doProcessEvent(gf::Event& event) {
    switch (event.type) {

    /*case gf::EventType::Resized: {
        auto size = m_game.getRenderer().getWindow().getSize();
        m_game.getRenderer().handleResize(size.x, size.y);
        break;
    }*/

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

void LobbyScene::doUpdate(gf::Time) {
    gf::Packet packet;

    // --- Réception réseau ---
    while (m_game.tryPopPacket(packet)) {
        switch (packet.getType()) {

            case ServerRoomSettings::type: {
                auto data = packet.as<ServerRoomSettings>();
                m_roomSettings = data.settings;
                m_entity.setRoomSettings(m_roomSettings);
                break;
            }

            case ServerListRoomPlayers::type: {
                auto data = packet.as<ServerListRoomPlayers>();
                m_players = data.players;
                m_entity.setPlayers(m_players);
                m_entity.setClientId(m_game.getMyId());
                break;
            }

            case ServerLeaveRoom::type:
                gf::Log::info("Serveur: quitté la room\n");
                m_game.requestScene(SceneRequest::GoToLobbyList);
                break;

            case ServerGameStart::type: {
                auto data = packet.as<ServerGameStart>();
                
                if (data.holeLinks.size() != 0) {
                    gf::Log::info(" portal trouvé, holeLinks pas vide!\n");
                } else {
                    gf::Log::info("Pas de portal dans network!\n");
                }
                
                gf::Log::info("Game start reçu -> passage en Playing\n");
                
                //Passer les données à GameScene AVANT de changer de scène
                m_game.goToGameScene(data.players, data.board, data.holeLinks);
                break;
            }

            default:
                break;
        }
    }

    LobbyAction act = m_entity.getAndResetLastAction();
    switch (act) {
        case LobbyAction::Leave: {
            gf::Packet p;
            p.is(ClientLeaveRoom{});
            m_game.getSocket().sendPacket(p);
            break;
        }

        case LobbyAction::RoomDec:
            if (m_roomSettings.roomSize > MIN_NB_PLAYERS) {
                RoomSettings s = m_roomSettings;
                s.roomSize--;
                gf::Packet p; p.is(ClientChangeRoomSettings{s});
                m_game.getSocket().sendPacket(p);
            }
            break;

        case LobbyAction::RoomInc:
            if (m_roomSettings.roomSize < MAX_NB_PLAYERS) {
                RoomSettings s = m_roomSettings;
                s.roomSize++;
                gf::Packet p; p.is(ClientChangeRoomSettings{s});
                m_game.getSocket().sendPacket(p);
            }
            break;

        case LobbyAction::BotDec:
            if (m_roomSettings.nbBot > MIN_NB_BOTS) {
                RoomSettings s = m_roomSettings;
                s.nbBot--;
                gf::Packet p; p.is(ClientChangeRoomSettings{s});
                m_game.getSocket().sendPacket(p);
            }
            break;

        case LobbyAction::BotInc:
            if (m_roomSettings.nbBot < MAX_NB_BOTS) {
                RoomSettings s = m_roomSettings;
                s.nbBot++;
                gf::Packet p; p.is(ClientChangeRoomSettings{s});
                m_game.getSocket().sendPacket(p);
            }
            break;

        case LobbyAction::DurDec:
            if (m_roomSettings.gameDuration > MIN_DURATION) {
                RoomSettings s = m_roomSettings;
                s.gameDuration -= 20;
                gf::Packet p; p.is(ClientChangeRoomSettings{s});
                m_game.getSocket().sendPacket(p);
            }
            break;

        case LobbyAction::DurInc:
            if (m_roomSettings.gameDuration < MAX_DURATION) {
                RoomSettings s = m_roomSettings;
                s.gameDuration += 20;
                gf::Packet p; p.is(ClientChangeRoomSettings{s});
                m_game.getSocket().sendPacket(p);
            }
            break;

        case LobbyAction::ToggleReady: {
            m_amReady = !m_amReady;
            gf::Packet p; p.is(ClientReady{m_amReady});
            m_game.getSocket().sendPacket(p);
            break;
        }

        case LobbyAction::ChangeRole: {
            PlayerData d{};
            d.id = m_game.getMyId();
            d.ready = m_amReady;
            gf::Packet p;
            p.is(ClientChangeRoomCharacterData{d});
            m_game.getSocket().sendPacket(p);
            break;
        }

        default:
            break;
    }
}


void LobbyScene::resizeYourself(){
    auto size = m_game.getWindow().getSize(); 
    m_game.handleResize(size.x, size.y);
    getWorldView() = m_game.getMainView();
}