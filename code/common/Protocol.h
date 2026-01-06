#pragma once
#include <cstdint>
#include <vector>
#include <gf/Id.h>
#include <gf/SerializationOps.h>
#include <gf/Array2D.h>
#include "Types.h"

using namespace gf::literals;


/*
Liste des échanges client <-> serveur

Client (envoyé)
- Tentative de connexion au lobby
- Tentative de modification des paramètres de la partie
- Tentative de démarrage de la partie
- Tentative de déplacement
- Tentative d'activation de pouvoir


Serveur (envoyé)
- Réponse à la tentative de connexion au lobby
- Nouveau joueur dans le lobby (Tous les client)

- Réponse à la tentative de modification des paramètres
- Modification des paramètres de la partie (Tous les client)

- Réponse à la tentative de démarrage de la partie
- Démarrage de la partie (Tous les client)

- Réponde à la tentative de déplacement (Ex : s'il a ramassé un pouvoir / toucher qlq)
- Déplacement d'un joueur (Tous les client)
-- Joueur x mangé (Tous les client)
-- Pacgomme mangé (Tous les client)

- Réponse à la tentative d'activation d'un pouvoir
- Activation d'un pouvoir (Tous les client)

- Modification de l'environnement (destruction de mur) (Tous les client) ?

- Fin de partie (Tous les client)


*/
struct PlayerData {
    static constexpr gf::Id type = "PlayerData"_id;
    uint32_t id;
    float x, y;
    uint32_t color; // RGBA
    std::string name;
    PlayerRole role;
    int score;
    bool ready = false;
};
template<typename Archive>
  Archive& operator|(Archive& ar, PlayerData& data) {
    return ar | data.id | data.x | data.y | data.color | data.name | data.role | data.score| data.ready;
}
struct GameSettigns {
    static constexpr gf::Id type = "GameSettigns"_id;
    int duration = 0;
    std::vector<PlayerData> players;
};
template<typename Archive>
  Archive& operator|(Archive& ar, GameSettigns& data) {
    return ar | data.duration | data.players;
}



//Un peu inspiré de https://github.com/Hatunruna/ggj2020/blob/master/code/bits/common/Protocol.h


//Serveur -> client

struct ServerJoinRoom {
    static constexpr gf::Id type = "ServerJoinRoom"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ServerJoinRoom& data) {
    return ar;
}
struct ServerLeaveRoom {
    static constexpr gf::Id type = "ServerLeaveRoom"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ServerLeaveRoom& data) {
    return ar;
}

struct ServerChangeRole {
    static constexpr gf::Id type = "ServerChangeRole"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ServerChangeRole& data) {
    return ar;
}

//Les autres changement

struct ServerReady {
    static constexpr gf::Id type = "ServerReady"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ServerReady& data) {
    return ar;
}

struct ServerGameStart {
    static constexpr gf::Id type = "ServerGameStart"_id;
    GameSettigns settings;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ServerGameStart& data) {
    return ar | data.settings;
}
struct ServerGameOver {
    static constexpr gf::Id type = "ServerGameOver"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ServerGameOver& data) {
    return ar;
}

//Client -> serveur

struct ClientJoinRoom {
    static constexpr gf::Id type = "ClientJoinRoom"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ClientJoinRoom& data) {
    return ar;
}

struct ClientLeaveRoom {
    static constexpr gf::Id type = "ClientLeaveRoom"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ClientLeaveRoom& data) {
    return ar;
}

struct ClientChangeRole {
    static constexpr gf::Id type = "ClientChangeRole"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ClientChangeRole& data) {
    return ar;
}

struct ClientReady {
    static constexpr gf::Id type = "ClientReady"_id;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ClientReady& data) {
    return ar;
}

//

struct ClientMove {
    static constexpr gf::Id type = "ClientMove"_id;
    char moveDir;
};
template<typename Archive>
  Archive& operator|(Archive& ar, ClientMove& data) {
    return ar | data.moveDir;
}

struct CaseCommon {
    static constexpr gf::Id type = "CaseCommon"_id;
    CellType celltype;
    bool pacGomme = false;
    CaseCommon() : celltype(CellType::Floor) {}
    CaseCommon(CellType t, bool pg) {celltype = t; pacGomme = pg;}
};
template<typename Archive>
  Archive& operator|(Archive& ar, CaseCommon& data) {
    return ar | data.celltype | data.pacGomme;
}

struct BoardCommon {
    BoardCommon() {};
    BoardCommon(unsigned int w, unsigned int h) : grid({w, h}), width(w), height(h) {}
    static constexpr gf::Id type = "BoardCommon"_id;
    unsigned int width;
    unsigned int height;
    gf::Array2D<CaseCommon> grid;
};
template<typename Archive>
  Archive& operator|(Archive& ar, BoardCommon& data) {
    return ar | data.width | data.height | data.grid;
}


struct GameState {
    static constexpr gf::Id type = "GameState"_id;
    std::vector<PlayerData> clientStates;
    BoardCommon board;
};
template<typename Archive>
  Archive& operator|(Archive& ar, GameState& data) {
    return ar | data.clientStates | data.board;
}


