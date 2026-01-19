#pragma once

#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <atomic>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include "ClientScreen.h"
#include "LobbyListScene.h"
#include "../common/Types.h"
#include "../common/Protocol.h"

enum class ClientScreen;

void handleNetworkPackets(
    std::queue<gf::Packet>& packetQueue,
    std::mutex& packetMutex,
    ClientScreen& screen,
    std::vector<PlayerData>& states,
    BoardCommon& board,
    std::vector<std::pair<Position, PacGommeType>>& pacgommes,
    unsigned int& timeLeft,
    int& timeLeftPre,
    std::vector<PlayerData>& lobbyPlayers,
    RoomSettings& roomSettings,
    LobbyListScene& lobbyListScene,
    uint32_t& myId,
    PlayerRole& myRole,
    int& lastScore,
    GameEndReason& endReason,
    bool amReady
);


std::thread startNetworkReceiver(
    gf::TcpSocket& socket,
    std::atomic<bool>& running,
    std::queue<gf::Packet>& packetQueue,
    std::mutex& packetMutex
);
