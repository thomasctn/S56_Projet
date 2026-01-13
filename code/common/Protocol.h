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

//Faudra probablement les déplacer ces structures
struct Position
{
  static constexpr gf::Id type = "Position"_id;
  unsigned int x;
  unsigned int y;
  Position()
  {
    x = 0;
    y = 0;
  }
  Position(unsigned int xp, unsigned int yp)
  {
    x = xp;
    y = yp;
  }
  bool operator==(const Position &a) const
  {
    return (x == a.x && y == a.y);
  }
  bool operator<(const Position &a) const
  {
    return (x < a.x || (x == a.x && y < a.y));
  }
};
template <typename Archive>
Archive &operator|(Archive &ar, Position &data)
{
  return ar | data.x | data.y;
}

struct CaseCommon
{
  static constexpr gf::Id type = "CaseCommon"_id;
  CellType celltype;
  bool pacGomme = false;
  CaseCommon() : celltype(CellType::Floor) {}
  CaseCommon(CellType t)
  {
    celltype = t;
  }
};
template <typename Archive>
Archive &operator|(Archive &ar, CaseCommon &data)
{
  return ar | data.celltype;
}

struct BoardCommon
{
  BoardCommon() {};
  BoardCommon(unsigned int w, unsigned int h) : grid({w, h}), width(w), height(h) {}
  static constexpr gf::Id type = "BoardCommon"_id;
  unsigned int width;
  unsigned int height;
  gf::Array2D<CaseCommon> grid;
};
template <typename Archive>
Archive &operator|(Archive &ar, BoardCommon &data)
{
  return ar | data.width | data.height | data.grid;
}

struct PlayerData
{
  static constexpr gf::Id type = "PlayerData"_id;
  uint32_t id;
  float x, y;
  uint32_t color; // RGBA
  std::string name;
  PlayerRole role;
  int score;
  bool ready = false;
};
template <typename Archive>
Archive &operator|(Archive &ar, PlayerData &data)
{
  return ar | data.id | data.x | data.y | data.color | data.name | data.role | data.score | data.ready;
}

struct RoomSettings
{
  static constexpr gf::Id type = "RoomSettings"_id;
  unsigned int roomSize;
  unsigned int nbBot;
  unsigned int gameDuration;
};
template <typename Archive>
Archive &operator|(Archive &ar, RoomSettings &data)
{
  return ar | data.roomSize | data.nbBot | data.gameDuration;
}


//------------------------------------------

// Un peu inspiré de https://github.com/Hatunruna/ggj2020/blob/master/code/bits/common/Protocol.h

// Serveur -> client

//Réponse au client qui rejoind la room
struct ServerJoinRoom
{
  static constexpr gf::Id type = "ServerJoinRoom"_id;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerJoinRoom &data)
{
  return ar;
}
//Réponse au client qui quitte la room
struct ServerLeaveRoom
{
  static constexpr gf::Id type = "ServerLeaveRoom"_id;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerLeaveRoom &data)
{
  return ar;
}

//Réponse au client qui change de rôle/nom/couleur
struct ServerChangeRoomCharacterData
{
  static constexpr gf::Id type = "ServerChangeRoomCharacterData"_id;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerChangeRoomCharacterData &data)
{
  return ar;
}

// Réponse à tous les clients sur les données des joueurs (nom, rôles, couleurs, etc..)
struct ServerListRoomPlayers
{
  static constexpr gf::Id type = "ServerListRoomPlayers"_id;
  std::vector<PlayerData> players;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerListRoomPlayers &data)
{
  return ar | data.players;
}

//Réponse au client qui change les paramètres de la room
struct ServerChangeRoomSettings
{
  static constexpr gf::Id type = "ServerChangeRoomSettings"_id;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerChangeRoomSettings &data)
{
  return ar;
}

//Réponse à tous les clients sur les changement de paramètre de la room
struct ServerRoomSettings
{
  static constexpr gf::Id type = "ServerRoomSettings"_id;
  RoomSettings settings;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerRoomSettings &data)
{
  return ar | data.settings;
}
//Réponse au client qui est prêt
struct ServerReady
{
  static constexpr gf::Id type = "ServerReady"_id;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerReady &data)
{
  return ar;
}
//Réponse à tous les clients sur le démarrage de la partie
struct ServerGameStart
{
  static constexpr gf::Id type = "ServerGameStart"_id;
  BoardCommon board;
  std::vector<PlayerData> players;
  std::set<Position> pacgommes;
  RoomSettings settings;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerGameStart &data)
{
  return ar | data.board | data.players | data.pacgommes | data.settings;
}
//Réponse à tous les clients sur la mise à jour de la partie
struct ServerGameState
{
  static constexpr gf::Id type = "ServerGameState"_id;
  std::vector<PlayerData> clientStates;
  BoardCommon board;
  std::set<Position> pacgommes;
  unsigned int timeLeft;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerGameState &data)
{
  return ar | data.clientStates | data.board | data.pacgommes | data.timeLeft;
}

//Réponse à tous les clients sur la fin de la partie
struct ServerGameEnd
{
  static constexpr gf::Id type = "ServerGameEnd"_id;
};
template <typename Archive>
Archive &operator|(Archive &ar, ServerGameEnd &data)
{
  return ar;
}
struct ServerDisconnect
{
  static constexpr gf::Id type = "ServerDisconnect"_id;
};

template <typename Archive>
Archive &operator|(Archive &ar, ServerDisconnect &)
{
  return ar;
};

// Client -> serveur

struct ClientJoinRoom
{
  static constexpr gf::Id type = "ClientJoinRoom"_id;
  gf::Id room;
};
template <typename Archive>
Archive &operator|(Archive &ar, ClientJoinRoom &data)
{
  return ar | data.room;
}

struct ClientLeaveRoom
{
  static constexpr gf::Id type = "ClientLeaveRoom"_id;
};
template <typename Archive>
Archive &operator|(Archive &ar, ClientLeaveRoom &data)
{
  return ar;
};
//Requête au serveur pour le changement de données du personnage du joueur
struct ClientChangeRoomCharacterData
{
  static constexpr gf::Id type = "ClientChangeRoomCharacterData"_id;
  PlayerData newPlayerRoomData;
};
template <typename Archive>
Archive &operator|(Archive &ar, ClientChangeRoomCharacterData &data)
{
  return ar | data.newPlayerRoomData;
}
//Requête au serveur pour le changement des paramètres du jeu
struct ClientChangeRoomSettings
{
  static constexpr gf::Id type = "ClientChangeRoomSettings"_id;
  RoomSettings newSettings;
};
template <typename Archive>
Archive &operator|(Archive &ar, ClientChangeRoomSettings &data)
{
  return ar | data.newSettings;
}

struct ClientReady
{
  static constexpr gf::Id type = "ClientReady"_id;
  bool ready;
};
template <typename Archive>
Archive &operator|(Archive &ar, ClientReady &data)
{
  return ar | data.ready;
}

struct ClientDisconnect
{
  static constexpr gf::Id type = "ClientDisconnect"_id;
};
template <typename Archive>
Archive &operator|(Archive &ar, ClientDisconnect &data)
{
  return ar;
}

//

struct ClientMove
{
  static constexpr gf::Id type = "ClientMove"_id;
  char moveDir;
};
template <typename Archive>
Archive &operator|(Archive &ar, ClientMove &data)
{
  return ar | data.moveDir;
}
