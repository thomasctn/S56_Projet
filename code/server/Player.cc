#include "Player.h"
#include <gf/Log.h>
#include "../common/Constants.h"

// ------------------
// Constructeur
// ------------------
Player::Player(uint32_t id_, PlayerRole role_, const std::string& name_)
: id(id_)
, role(role_)
, x(0.0f)
, y(0.0f)
, color(0xFFFFFFFF)
, score(0)
, ready(false)
, name(name_)
, moveAccumulator(0.0)
, bufferedDir(Direction::None)
, powerTimeRemaining(0.0)
{
    switch (role) {
        case PlayerRole::PacMan:
            moveRate = PACMAN_SPEED_MOVERATE;
            isVunerable = true;
            break;
        case PlayerRole::Ghost:
            moveRate = GHOST_SPEED_MOVERATE;
            isVunerable = false;
            break;
        default:
            moveRate = DEFAULT_SPEED_MOVERATE;
            isVunerable = false;
            break;
    }
}


// ------------------
// Accesseurs rôle
// ------------------
PlayerRole Player::getRole() const {
    return role;
}

void Player::setRole(PlayerRole r) {
    role = r;
}

// ------------------
// État réseau
// ------------------
PlayerData Player::getState() const {
    return { id, x, y, color, "moi", role, score, true };
}

void Player::setState(const PlayerData& state) {
    x = state.x;
    y = state.y;
    color = state.color;
}

// ------------------
// Gameplay : manger
// ------------------
bool Player::eat(std::optional<PacGommeType> pacGommeType, Player* otherPlayer) {
    switch (role) {
        case PlayerRole::PacMan:
            if (pacGommeType.has_value()) {
                if (pacGommeType.value() == PacGommeType::Basic) {
                    score += 10;
                    gf::Log::info("PacMan %d a mangé une pac-gomme normale. Score=%d\n", id, score);
                } else if (pacGommeType.value() == PacGommeType::Power) {
                    score += 50;
                    isVunerable = false;
                    powerTimeRemaining = CHASSEUR_MODE_TIME; // 10 secondes de mode chasse
                    gf::Log::info("PacMan %d a mangé une PAC-GOMME POWER !\n", id);
                }
                return true;
            }
            break;

        case PlayerRole::Ghost:
            if (otherPlayer && otherPlayer->getRole() == PlayerRole::PacMan) {
                if (otherPlayer->isVunerable) {
                    // Respawn PacMan si vulnérable
                    otherPlayer->x = 50.0f;
                    otherPlayer->y = 50.0f;
                    otherPlayer->score = 0;
                    gf::Log::info("Fantôme %d a mangé PacMan %d\n", id, otherPlayer->id);
                    return true;
                } else {
                    // PacMan est en mode power : il peut manger le fantôme !
                    otherPlayer->score += 100;
                    x = 50.0f; // respawn du fantôme
                    y = 50.0f;
                    gf::Log::info("PacMan %d a mangé Fantôme %d !\n", otherPlayer->id, id);
                    return true;
                }
            }
            break;

        default:
            break;
    }
    return false;
}

void Player::update(double dt) {
    if (!isVunerable) {
        powerTimeRemaining -= dt;
        if (powerTimeRemaining <= 0.0) {
            isVunerable = true;
            powerTimeRemaining = 0.0;
            gf::Log::info("PacMan %d n'est plus en mode chasseur !\n", id);
        }
    }
}

