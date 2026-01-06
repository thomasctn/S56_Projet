#pragma once

#include <thread>
#include <chrono>
#include <unordered_map>
#include <memory>
#include <gf/Log.h>
#include <atomic> 

#include "Player.h"
#include "Plateau.h"
#include "../common/Constants.h"

enum class Direction {
    Up, Down, Left, Right
};

class Game {
public:
    Game(int width, int height);

    // --- Joueur ---
    bool requestMove(uint32_t playerId, Direction dir);
    Player& getPlayerInfo(uint32_t playerId);
    void addPlayer(uint32_t id, float x, float y);
    bool canMove(uint32_t playerId, float newX, float newY) const;
    void spawnPlayer(Player& p);
    
    std::unordered_map<uint32_t, std::unique_ptr<Player>>& getPlayers() { return players; }
    const std::unordered_map<uint32_t, std::unique_ptr<Player>>& getPlayers() const { return players; }


    // --- Plateau ---
    Plateau& getPlateau() { return plateau; }
    const Plateau& getPlateau() const { return plateau; }

    // --- Chrono ---
    double getElapsedSeconds() const { return gameElapsed; }
    double getPreGameElapsed() const { return preGameElapsed; }


    void startGameLoop(int tickMs_);
    void stopGameLoop();
    bool isGameStarted() const { return gameStarted; }
    bool isPreGame() const { return !gameStarted && preGameElapsed < preGameDelay; }
    bool isGameOver() const { return gameStarted && gameElapsed >= T_GAME; }

    void startChrono();
    void resetChrono();

    
private:
    Plateau plateau;
    std::unordered_map<uint32_t, std::unique_ptr<Player>> players;

    // --- Chrono ---
    std::chrono::steady_clock::time_point chronoStart;
    std::chrono::steady_clock::time_point preGameStart;

    
    double preGameElapsed = 0.0;
    double gameElapsed = 0.0;
    const int preGameDelay = PRE_GAME_DELAY;

    // --- Boucle de jeu ---
    int tickMs = 50;
    std::thread gameThread;
    std::atomic<bool> running{false};
    std::atomic<bool> gameStarted{false};
};
