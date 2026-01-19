#pragma once

#include <gf/Event.h>
#include <gf/Action.h>
#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <atomic>
#include <thread>
#include "Renderer.h"
#include "WelcomeScene.h"
#include "LobbyScene.h"
#include "EndScene.h"
#include "ClientScreen.h"
#include "../common/Types.h"
#include "../common/Constants.h"


void handleClientEvents(
    Renderer& renderer,
    gf::ActionContainer& actions,
    std::atomic<bool>& running,
    ClientScreen& screen,
    WelcomeScene& welcomeScene,
    LobbyScene& lobbyScene,
    EndScene& endScene,
    gf::TcpSocket& socket,
    bool& askedToJoin,
    bool& amReady,
    int& roomSize,
    int& nbBots,
    int& gameDur,
    PlayerRole& myRole,
    uint32_t myId,
    std::thread& receiver
);

