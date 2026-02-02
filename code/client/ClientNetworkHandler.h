#pragma once

#include <gf/TcpSocket.h>
#include <gf/Packet.h>
#include <atomic>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include "LobbyListScene.h"
#include "../common/Types.h"
#include "../common/Protocol.h"

enum class ClientScreen;




std::thread startNetworkReceiver(
    gf::TcpSocket& socket,
    std::atomic<bool>& running,
    std::queue<gf::Packet>& packetQueue,
    std::mutex& packetMutex
);
