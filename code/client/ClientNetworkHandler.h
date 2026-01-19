#pragma once

#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <atomic>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include "ClientScreen.h"
#include "../common/Types.h"
#include "../common/Protocol.h"

enum class ClientScreen;

void handleNetworkPackets(
    std::queue<gf::Packet>& packetQueue,
    std::mutex& packetMutex,
    ClientScreen& screen,
    std::vector<PlayerData>& states,
    BoardCommon& board,
    std::set<Position>& pacgommes,
    unsigned int& timeLeft,
    int& timeLeftPre,
    int& connectedPlayers,
    int& roomSize,
    int& nbBots,
    int& gameDur,
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
