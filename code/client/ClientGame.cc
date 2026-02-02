#include "ClientGame.h"
#include "GameScene.h"


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
, lobbyListScene(*this) 
, lobbyScene(*this)
, gameScene(*this)
, endScene(*this)
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



gf::TcpSocket& ClientGame::getSocket() {
    return m_socket;
}
void ClientGame::setMyId(uint32_t id) {
    myId = id;
}

uint32_t ClientGame::getMyId() const {
    return myId;
}

void ClientGame::requestScene(SceneRequest req) {
    switch (req) {
        case SceneRequest::GoToLobbyList:
            // remplace la pile entière par la scène LobbyList
            replaceAllScenes(lobbyListScene);
            lobbyListScene.getEntity().setRooms(m_lastRooms);
            
            break;

        case SceneRequest::GoToLobby:
            replaceAllScenes(lobbyScene);
            break;

          

        default:
            break;
    }
}

void ClientGame::goToGameScene(const std::vector<PlayerData>& players,const BoardCommon& board,const std::map<Position, Position>& holeLinks){
    gameScene.setInitialState(players, board, holeLinks);
    replaceAllScenes(gameScene);

}

void ClientGame::goToEndScene(GameEndReason& endReason){
    endScene.initEnd(endReason, lastScore);
    replaceAllScenes(endScene);
}





//sort juste un packet
bool ClientGame::tryPopPacket(gf::Packet& out) {
    std::lock_guard<std::mutex> lock(m_packetMutex);
    if (m_packetQueue.empty()) return false;
    out = std::move(m_packetQueue.front());
    m_packetQueue.pop();
    return true;
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

void ClientGame::startNetwork(const std::string& host, const std::string& port) {
    m_running.store(true);
    connectToServer(host, port);
    m_socket.setNonBlocking();
    m_receiver = startNetworkReceiver(m_socket, m_running, m_packetQueue, m_packetMutex);
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

void ClientGame::shutdown() {
    if (!m_running.load()) {
        return;
    }

    gf::Log::info("Shutdown client demandé\n");
    popAllScenes();   
}



void ClientGame::run(const std::string& host, const std::string& port) {
    gf::Log::info("Démarrage ClientGame::run()\n");

    startNetwork(host, port); //debut reseau

    pushScene(welcomeScene); //premiere scene

    gf::SceneManager::run(); //on run

    stopNetwork(); //on ferme

    gf::Log::info("ClientGame terminé proprement\n");
}





/*comment le resize fonctionnait:
if (event.type == gf::EventType::Resized) { //changement taille
                    auto size = renderer.getWindow().getSize();
                    renderer.handleResize(size.x, size.y);
                    //gf::Log::info("Handleresize appelé\n");

                }*/