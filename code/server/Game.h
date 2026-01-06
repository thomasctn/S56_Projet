#pragma once


#include <thread>
#include <atomic>
#include <chrono>
#include <stdexcept>
#include <vector>
#include <gf/Log.h>


#include "Player.h"
#include "Plateau.h"


enum class Direction {
    Up,
    Down,
    Left,
    Right
};


class Game {
public:
    Game(int width, int height);

    // --- Joueur ---
    void requestMove(uint32_t playerId, Direction dir);
    Player& getPlayerInfo(uint32_t playerId);
    void addPlayer(uint32_t id, float x, float y);
    bool canMove(uint32_t playerId, float newX, float newY) const;
    void spawnPlayer(Player& p);


    // --- Plateau ---
    Plateau& getPlateau() { return plateau; }
    const Plateau& getPlateau() const { return plateau; }
    std::unordered_map<uint32_t, std::unique_ptr<Player>>& getPlayers() {
        return players;
    }
    const std::unordered_map<uint32_t, std::unique_ptr<Player>>& getPlayers() const {
        return players;
    }

    // --- Chrono ---
    void startChrono();
    double getElapsedSeconds() const;
    void resetChrono();

    // --- Boucle de jeu ---
    void startGameLoop(int tickMs = 50);
    void stopGameLoop();
    bool isRunning() const { return running; }

private:
    Plateau plateau;
    std::unordered_map<uint32_t, std::unique_ptr<Player>> players;

    // Chrono
    std::chrono::steady_clock::time_point chronoStart;

    // Boucle de jeu
    std::thread gameThread;
    std::atomic<bool> running{false};
    int tickMs{50};
};
