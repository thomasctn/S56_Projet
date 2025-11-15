#include "GameNetworkServer.h"
#include <gf/Log.h>

int main() {
    gf::Log::info("Démarrage du serveur...");
    GameNetworkServer server;
    return server.run();
}
