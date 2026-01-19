// Constants.h
#pragma once
#include <cstddef>
// Savoir si on dev. A DESACTIVER en cas PUBLICATION sur un SERVER
constexpr bool DEV = true;


constexpr int T_GAME = 60;      // durée de la partie en secondes
constexpr int PRE_GAME_DELAY = 5; // délai avant début

constexpr int SPEED = 30; // vitesse du serv

constexpr size_t MAX_PLAYERS = 2; //nombre de joueur par room
constexpr size_t NB_BOTS = 4; //nombre de Bot par partie
constexpr unsigned int NB_PACGOMME = 25; //nombre de pacgomme


static constexpr int VISION_RANGE = 4;
static constexpr float MOVE_INTERVAL = 0.5f;
static constexpr float CASE_SIZE = 50;

constexpr int MIN_NB_PLAYERS = 1;
constexpr int MAX_NB_PLAYERS = 5;
constexpr int MIN_NB_BOTS = 0;
constexpr int MAX_NB_BOTS = 8 ;
constexpr int MIN_DURATION = 60; //60 mais 10 pour test
constexpr int MAX_DURATION =300;


static constexpr float PACMAN_ATTRACTION        = 5.0f;
static constexpr float OTHER_GHOST_REPULSION    = 3.0f;
static constexpr float SELF_GHOST_REPULSION     = 1.0f;
static constexpr float RANDOM_NOISE             = 0.2f;
static constexpr float DECAY_FACTOR             = 0.95f;

