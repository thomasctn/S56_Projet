#include "ClientGame.h"

#include <gf/Log.h>
#include <chrono>
#include <thread>

ClientGame::ClientGame()
: gf::SceneManager("ClientGame", gf::vec(1280, 720))
, upAction("Up")
, downAction("Down")
, leftAction("Left")
, rightAction("Right")
, welcomeScene(*this)
, lobbyListScene(m_renderer) //ou *this?
, lobbyScene(m_renderer)
, gameScene(m_renderer)
, endScene(m_renderer)
, m_running(false)
, roomSettings{ unsigned(MAX_PLAYERS), unsigned(NB_BOTS), unsigned(T_GAME) }
{
    upAction.addKeycodeKeyControl(gf::Keycode::Up);
    upAction.setContinuous();
    actions.addAction(upAction);

    downAction.addKeycodeKeyControl(gf::Keycode::Down);
    downAction.setContinuous();
    actions.addAction(downAction);

    leftAction.addKeycodeKeyControl(gf::Keycode::Left);
    leftAction.setContinuous();
    actions.addAction(leftAction);

    rightAction.addKeycodeKeyControl(gf::Keycode::Right);
    rightAction.setContinuous();
    actions.addAction(rightAction);
}

ClientGame::~ClientGame() {
    stopNetwork();
}
void shutdownClient(std::atomic<bool>& running)
{
    if (!running.load()) return;

    gf::Log::info("Arrêt du client...\n");
    running.store(false);
}


Renderer& ClientGame::getRenderer() {
    return m_renderer;
}

gf::TcpSocket& ClientGame::getSocket() {
    return m_socket;
}

void ClientGame::connectToServer(const std::string& host, const std::string& port) {
    bool connected = false;
    while (!connected && m_running.load()) {
        m_socket = gf::TcpSocket(host.c_str(), port.c_str());
        if (m_socket) {
            connected = true;
            gf::Log::info("Connecté au serveur !\n");
        } else {
            gf::Log::info("Impossible de se connecter au serveur, nouvelle tentative dans 1s...\n");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void ClientGame::stopNetwork() {
    if (m_receiver.joinable()) {
        m_running.store(false);
        m_receiver.join();
    }
    //fermer socket proprement
    if (m_socket) {
        gf::TcpSocket tmp = std::move(m_socket);
    }
}

void ClientGame::run(const std::string& host, const std::string& port) {
    gf::Log::info("Démarrage ClientGame::run()\n");

    gf::Log::info("Up active? %d\n", actions.getAction("Up").isActive());


    m_running.store(true);

    //connexion au serveur (bloquant tant que non connecté)
    connectToServer(host, port);

    //set non blocking and start receiver thread using your helper
    m_socket.setNonBlocking();

    //start network receiver (uses the free function you already have)
    m_receiver = startNetworkReceiver(
        m_socket,
        m_running,
        m_packetQueue,
        m_packetMutex
    );

    auto lastSend = std::chrono::steady_clock::now();
    gf::Log::info("Avant boucle principale\n");

    while (m_running.load()) {
        //gf::Log::info("Loop \n");

        handleClientEvents(
            m_renderer,
            actions,
            m_running,
            screen,
            welcomeScene,
            lobbyScene,
            endScene,
            lobbyListScene,
            m_socket,
            askedToJoin,
            amReady,
            roomSettings,
            myRole,
            myId,
            m_receiver
        );
        //gf::Log::info("Après handleClientEvents\n");


        if (!m_running.load()) break;

        if (screen == ClientScreen::Playing) {
            auto now = std::chrono::steady_clock::now();
            if (now - lastSend > std::chrono::milliseconds(1000 / CLIENT_MOVE_SPEED)) {
                char dir = 0;
                if (actions.getAction("Up").isActive()) dir = 'U';
                else if (actions.getAction("Down").isActive()) dir = 'D';
                else if (actions.getAction("Left").isActive()) dir = 'L';
                else if (actions.getAction("Right").isActive()) dir = 'R';

                if (dir != 0) {
                    gf::Log::info("Envoi : touche '%c'\n", dir);
                    gf::Packet packet;
                    ClientMove cm;
                    cm.moveDir = dir;
                    packet.is(cm);
                    m_socket.sendPacket(packet);
                    lastSend = now;
                }
            }
        }

        handleNetworkPackets(
            m_packetQueue,
            m_packetMutex,
            screen,
            states,
            board,
            pacgommes,
            timeLeft,
            timeLeftPre,
            lobbyPlayers,
            roomSettings,
            lobbyListScene,
            myId,
            myRole,
            lastScore,
            endReason,
            amReady,
            holeLinks
        );

        m_renderer.clearWindow();
        m_renderer.getRenderWindow().display();
         /*if (screen == ClientScreen::Welcome) {
            welcomeScene.render();
         }   */
        if (screen == ClientScreen::Lobby) {
            lobbyScene.render(lobbyPlayers, roomSettings, myId);
        } else if (screen == ClientScreen::End) {
            endScene.render(lastScore, endReason);
        } else if (screen == ClientScreen::LobbyList) {
            lobbyListScene.render();
        } else { // Playing
            gameScene.render(states, myId, board, pacgommes, timeLeftPre, timeLeft, holeLinks);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    stopNetwork();
    if (m_renderer.getWindow().isOpen()) {
        m_renderer.getWindow().close();
    }

    gf::Log::info("ClientGame::run() terminé proprement\n");
}
