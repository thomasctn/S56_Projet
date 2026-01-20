#pragma once

#include <cstdint>
#include <gf/TcpSocket.h>

#include "../common/Protocol.h"
#include "../common/Types.h"


class Controller;

class Player {
public:
    Player(uint32_t id = 0, PlayerRole role = PlayerRole::Spectator, const std::string& name = "moi");

    PlayerRole getRole() const;
    void setRole(PlayerRole r);

    PlayerData getState() const;
    void setState(const PlayerData& state);

    bool eat(bool isPacGomme, Player* otherPlayer = nullptr);

    uint32_t getId() const { return id; }
    int getScore() const { return score; }
    void setScore(int s) { score = s; }

    bool isReady() const { return ready; }
    void setReady(bool r) { ready = r; }

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;
    Player(Player&&) = default;
    Player& operator=(Player&&) = default;

public:
    uint32_t id;
    PlayerRole role;
    float x = 0.0f;
    float y = 0.0f;
    uint32_t color = 0xFFFFFFFF;
    int score = 0;
    std::string name;
    Controller* controller = nullptr;
    bool isBot = false;

private:
    bool ready = false;
};

