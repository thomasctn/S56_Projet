#pragma once
#include <cstdint>
#include <vector>
#include <gf/Id.h>
#include <gf/SerializationOps.h>

enum class CellType { Floor, Wall, Hut};


enum class PlayerRole {
    PacMan,
    Ghost,
    Spectator
};

enum class Direction {
    Up,
    Down,
    Left,
    Right,
    None
};

enum class GameEndReason {
    ALL_DOT_EATEN,
    TIME_OUT,
    PACMAN_DEATH,
};

enum class PacGommeType {
    Basic,
    Power,    // rend Pac-Man chasseur
};