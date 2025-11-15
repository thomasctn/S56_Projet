#pragma once
#include <gf/TcpSocket.h>
#include "../common/Types.h"

struct ClientInfo {
    uint32_t id;
    gf::TcpSocket socket;
    ClientState state;
};
