// Constants.h
#pragma once
#include <cstddef>
// Savoir si on dev. A DESACTIVER en cas PUBLICATION sur un SERVER
constexpr bool DEV = false;


constexpr int T_GAME                            = 60;      // durée de la partie en secondes
constexpr int PRE_GAME_DELAY                    = 5; // délai avant début

constexpr int SPEED = 30; // vitesse du serv
constexpr int CLIENT_MOVE_SPEED = 10; // vitesse de déplacement du client

constexpr size_t MAX_PLAYERS                    = 2;
constexpr size_t NB_BOTS                        = 4;
constexpr unsigned int PACGOMME_COUNT           = 40;
constexpr unsigned int POWER_PACGOMME_COUNT     = 4;

static constexpr int VISION_RANGE_BOTS          = 6;
static constexpr float MOVE_INTERVAL            = 0.5f;
static constexpr float CASE_SIZE                = 50;

constexpr int MIN_NB_PLAYERS                    = 1;
constexpr int MAX_NB_PLAYERS                    = 5;
constexpr int MIN_NB_BOTS                       = 0;
constexpr int MAX_NB_BOTS                       = 8 ;
constexpr int MIN_DURATION                      = 120; //120 mais 10 pour test
constexpr int MAX_DURATION                      = 360;


static constexpr float PACMAN_ATTRACTION        = 5.0f;
static constexpr float OTHER_GHOST_REPULSION    = 3.0f;
static constexpr float SELF_GHOST_REPULSION     = 1.0f;
static constexpr float RANDOM_NOISE             = 0.2f;
static constexpr float DECAY_FACTOR             = 0.95f;

static constexpr float PACMAN_SPEED_MOVERATE    = 6.0f;
static constexpr float GHOST_SPEED_MOVERATE     = 4.0f;
static constexpr float DEFAULT_SPEED_MOVERATE   = 0.0f;

static constexpr float CHASSEUR_MODE_TIME       = 10.0;


