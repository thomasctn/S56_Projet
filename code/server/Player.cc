#include "Player.h"

// ------------------
// Constructeur
// ------------------
Player::Player(uint32_t id_, PlayerRole role_)
: id(id_)
, role(role_)
, x(0.0f)
, y(0.0f)
, color(0xFFFFFFFF)
, score(0)
{
}

// ------------------
// Rôle
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
ClientState Player::getState() const {
    return { id, x, y, color };
}

void Player::setState(const ClientState& state) {
    x = state.x;
    y = state.y;
    color = state.color;
}

// ------------------
// Gameplay : manger
// ------------------
bool Player::eat(bool isPacGomme, Player* otherPlayer) {
    switch (role) {
        case PlayerRole::PacMan:
            if (isPacGomme) {
                score += 10;
                gf::Log::info(
                    "PacMan %d a mangé une pac-gomme. Score=%d\n",
                    id, score
                );
                return true;
            }
            break;

        case PlayerRole::Ghost:
            if (otherPlayer && otherPlayer->getRole() == PlayerRole::PacMan) {
                // Respawn PacMan
                otherPlayer->x = 50.0f;
                otherPlayer->y = 50.0f;
                otherPlayer->score = 0;

                gf::Log::info(
                    "Fantôme %d a mangé PacMan %d\n",
                    id, otherPlayer->id
                );
                return true;
            }
            break;

        default:
            break;
    }

    return false;
}
