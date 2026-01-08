#pragma once

#include <gf/Log.h>

#include <thread>
#include <chrono>
#include <unordered_map>
#include <memory>
#include <atomic> 
#include <mutex>

#include "../common/Constants.h"
#include "Player.h"
#include "Board.h"
#include "InputQueue.h"

class Room;
class ServerNetwork;

class Game {
public:
    Game(int width, int height);

    // --- Joueur ---
    bool requestMove(uint32_t playerId, Direction dir);
    Player& getPlayerInfo(uint32_t playerId);
    void addPlayer(uint32_t id, float x, float y, PlayerRole role);
    bool canMove(uint32_t playerId, float newX, float newY) const;
    void spawnPlayer(Player& p);

    std::unordered_map<uint32_t, std::unique_ptr<Player>>& getPlayers() { return players; }
    const std::unordered_map<uint32_t, std::unique_ptr<Player>>& getPlayers() const { return players; }
    void removePlayer(uint32_t playerId);


    // --- Board ---
    Board& getBoard() { return board; }
    const Board& getBoard() const { return board; }

    // --- Chrono ---
    double getElapsedSeconds() const;
    double getPreGameElapsed() const;
    void startChrono();
    void resetChrono();

    // --- GameLoop ---
    void startGameLoop(int tickMs_, InputQueue& inputQueue, ServerNetwork& server);
    void stopGameLoop();
    bool isGameStarted() const { return gameStarted; }
    bool isPreGame() const;
    bool isGameOver() const;

    Room* room = nullptr;
    void setRoom(Room& r) { room = &r; }


    
private:
    Board board;
    std::unordered_map<uint32_t, std::unique_ptr<Player>> players;

    // --- Chrono ---
    std::chrono::steady_clock::time_point chronoStart;
    std::chrono::steady_clock::time_point preGameStart;

    mutable std::mutex chronoMutex;
    double preGameElapsed = 0.0;
    double gameElapsed = 0.0;
    const int preGameDelay = PRE_GAME_DELAY;

    // --- GameLoop ---
    int tickMs = 50;
    std::thread gameThread;
    std::atomic<bool> running{false};
    std::atomic<bool> gameStarted{false};

    void processInputs(InputQueue& queue);
};
