#include "GameEntity.h"
#include <gf/Text.h>
#include <gf/Font.h>
#include <gf/Log.h>
#include <gf/RenderTarget.h>
#include <gf/RenderStates.h>
#include <gf/Shapes.h>
#include <algorithm>

GameEntity::GameEntity()
: m_font("../common/fonts/arial.ttf")
{
    m_inkyTexture = gf::Texture("../client/assets/ghosts/inky.png");
    m_inkySprite.setTexture(m_inkyTexture);
    m_inkySprite.setOrigin({0.f, 0.f});

    m_clydeTexture = gf::Texture("../client/assets/ghosts/clyde.png");
    m_clydeSprite.setTexture(m_clydeTexture);
    m_clydeSprite.setOrigin({0.f, 0.f});

    m_pinkyTexture = gf::Texture("../client/assets/ghosts/pinky.png");
    m_pinkySprite.setTexture(m_pinkyTexture);
    m_pinkySprite.setOrigin({0.f, 0.f});

    m_blinkyTexture = gf::Texture("../client/assets/ghosts/blinky.png");
    m_blinkySprite.setTexture(m_blinkyTexture);
    m_blinkySprite.setOrigin({0.f, 0.f});

    m_pacmanRightTexture  = gf::Texture("../client/assets/pacman/right_pacman_sheet.png");
    m_pacmanLeftTexture   = gf::Texture("../client/assets/pacman/left_pacman_sheet.png");
    m_pacmanUpTexture     = gf::Texture("../client/assets/pacman/up_pacman_sheet.png");
    m_pacmanDownTexture   = gf::Texture("../client/assets/pacman/down_pacman_sheet.png");

    m_pacmanRightAnim.addTileset(m_pacmanRightTexture, {4,1}, gf::seconds(0.1f), 4);
    m_pacmanLeftAnim.addTileset(m_pacmanLeftTexture,  {4,1}, gf::seconds(0.1f), 4);
    m_pacmanUpAnim.addTileset(m_pacmanUpTexture,      {4,1}, gf::seconds(0.1f), 4);
    m_pacmanDownAnim.addTileset(m_pacmanDownTexture,  {4,1}, gf::seconds(0.1f), 4);

    m_pacmanRightAnim.setLoop(true);
    m_pacmanLeftAnim.setLoop(true);
    m_pacmanUpAnim.setLoop(true);
    m_pacmanDownAnim.setLoop(true);

    m_pacmanSprite.setAnimation(m_pacmanRightAnim);
    m_pacmanSprite.setOrigin({0.f, 0.f});
    m_pacmanDir = 'R';
}

void GameEntity::setGameState(const std::vector<PlayerData>& states) {
    m_states = states;
}

void GameEntity::setBoard(const BoardCommon& board) {
    m_board = board;
}

void GameEntity::setPacGommes(const std::vector<std::pair<Position, PacGommeType>>& pacgommes) {
    m_pacgommes = pacgommes;
}

void GameEntity::setTimeLeft(int timeLeftPre, unsigned int timeLeft) {
    m_timeLeftPre = timeLeftPre;
    m_timeLeft = timeLeft;
}

void GameEntity::setGameTimeLeft(unsigned int timeLeft) {
    m_timeLeft = timeLeft;
}

void GameEntity::setHoleLinks(const std::map<Position, Position>& holeLinks) {
    m_holeLinks = holeLinks;
}

void GameEntity::setClientId(uint32_t id) {
    m_clientId = id;
}

void GameEntity::update(gf::Time time) { //update anim pacman
    m_pacmanSprite.update(time);
}



void GameEntity::calculateMovement(gf::RenderTarget& target, const BoardCommon &map, float &tileSize, float &offsetX, float &offsetY) {
    gf::View view = target.getView();
    gf::Vector2f viewSize = view.getSize();

    float topMargin = std::min(viewSize.y * 0.16f, 120.0f); //espace ou on met le texte en haut
    float availW = viewSize.x;
    float availH = viewSize.y - topMargin;

    tileSize = std::min(availW / float(map.width), availH / float(map.height));
    offsetX = (viewSize.x - tileSize * float(map.width)) / 2.f + (view.getCenter().x - viewSize.x * 0.5f);
    offsetY = topMargin + (view.getCenter().y - viewSize.y * 0.5f);
}

void GameEntity::renderMap(gf::RenderTarget& target, const gf::RenderStates& states, const BoardCommon &map, float tileSize, float offsetX, float offsetY) {
    for (unsigned int y = 0; y < map.height; ++y) {
        for (unsigned int x = 0; x < map.width; ++x) {
            const CaseCommon& cell = map.grid({ x, y });

            gf::RectangleShape tile({tileSize, tileSize});
            tile.setPosition({x * tileSize + offsetX, y * tileSize + offsetY});

            switch (cell.celltype) {
                case CellType::Wall: tile.setColor(gf::Color::White); break;
                case CellType::Hut:  tile.setColor(gf::Color::Red); break;
                case CellType::Floor: tile.setColor(gf::Color::fromRgb(0.3f, 0.3f, 0.3f)); break;
                default: tile.setColor(gf::Color::Black); break;
            }

            target.draw(tile, states);
        }
    }

    if (m_holeLinks.size() >= 2) {
        auto it = m_holeLinks.begin();

        gf::RectangleShape portalTile({tileSize, tileSize});

        portalTile.setPosition({it->first.x * tileSize + offsetX, it->first.y * tileSize + offsetY});
        portalTile.setColor(gf::Color::Magenta);
        target.draw(portalTile, states);

        portalTile.setPosition({it->second.x * tileSize + offsetX, it->second.y * tileSize + offsetY});
        target.draw(portalTile, states);

        ++it;

        portalTile.setPosition({it->first.x * tileSize + offsetX, it->first.y * tileSize + offsetY});
        portalTile.setColor(gf::Color::Blue);
        target.draw(portalTile, states);

        portalTile.setPosition({it->second.x * tileSize + offsetX, it->second.y * tileSize + offsetY});
        target.draw(portalTile, states);
    } else {
        // gf::Log::info("Pas de portal!\n");
    }
}

void GameEntity::renderPacGommes(gf::RenderTarget& target, const gf::RenderStates& states, const std::vector<std::pair<Position, PacGommeType>>& pacgommes, float tileSize, float offsetX, float offsetY) {
    for (const auto& [pos, type] : pacgommes) {
        gf::CircleShape pacGomme(tileSize / 6.0f);
        pacGomme.setOrigin({tileSize / 12.0f, tileSize / 12.0f});
        pacGomme.setPosition({
            pos.x * tileSize + offsetX + tileSize / 2.0f,
            pos.y * tileSize + offsetY + tileSize / 2.0f
        });

        switch(type) {
            case PacGommeType::Basic:
                pacGomme.setColor(gf::Color::Yellow);
                break;
            case PacGommeType::Power:
                pacGomme.setColor(gf::Color::Green);
                break;
            default:
                pacGomme.setColor(gf::Color::White);
                break;
        }

        target.draw(pacGomme, states);
    }
}

void GameEntity::render(gf::RenderTarget& target, const gf::RenderStates& states) {
    
    if (m_board.width == 0 || m_board.height == 0) {
        return;
    }

    float tileSize = 0.f;
    float offsetX = 0.f;
    float offsetY = 0.f;
    calculateMovement(target, m_board, tileSize, offsetX, offsetY);

    renderMap(target, states, m_board, tileSize, offsetX, offsetY);

    renderPacGommes(target, states, m_pacgommes, tileSize, offsetX, offsetY);

    gf::Text timer;
    timer.setFont(m_font);
    timer.setCharacterSize(24);
    timer.setColor(gf::Color::White);

    if (m_timeLeftPre != 0) {

        timer.setString("Temps avant le début de la partie : " + std::to_string(m_timeLeftPre));
    } else {
        timer.setString("Temps avant la fin de la partie : " + std::to_string(m_timeLeft));
    }
    timer.setPosition({ offsetX, offsetY - 40.f });
    target.draw(timer, states);

    // Scale sprites pr tileSize
    auto texSize = m_inkyTexture.getSize();
    if (texSize.x > 0 && texSize.y > 0) {
        float scaleX = tileSize / float(texSize.x);
        float scaleY = tileSize / float(texSize.y);
        m_inkySprite.setScale({scaleX, scaleY});
        m_clydeSprite.setScale({scaleX, scaleY});
        m_pinkySprite.setScale({scaleX, scaleY});
        m_blinkySprite.setScale({scaleX, scaleY});
    }

    int ghostIndex = 0;

    for (const auto &s : m_states) {
        float px = s.x / 50.0f * tileSize + offsetX;
        float py = s.y / 50.0f * tileSize + offsetY;

        if (s.role == PlayerRole::PacMan) {
            if (m_hasLastPacmanPos) {
                if (px > m_lastPacmanX && m_pacmanDir != 'R') { m_pacmanDir = 'R'; m_pacmanSprite.setAnimation(m_pacmanRightAnim); }
                else if (px < m_lastPacmanX && m_pacmanDir != 'L') { m_pacmanDir = 'L'; m_pacmanSprite.setAnimation(m_pacmanLeftAnim); }
                else if (py > m_lastPacmanY && m_pacmanDir != 'D') { m_pacmanDir = 'D'; m_pacmanSprite.setAnimation(m_pacmanDownAnim); }
                else if (py < m_lastPacmanY && m_pacmanDir != 'U') { m_pacmanDir = 'U'; m_pacmanSprite.setAnimation(m_pacmanUpAnim); }
            }

            m_pacmanSprite.setPosition({px, py});

            auto pacTexSize = m_pacmanRightTexture.getSize();
            if (pacTexSize.x > 0) {
                float frameWidth  = pacTexSize.x / 4.0f;
                float frameHeight = pacTexSize.y;
                m_pacmanSprite.setScale({tileSize / frameWidth, tileSize / frameHeight});
            }

            target.draw(m_pacmanSprite, states);

            //Score
            gf::Text scoreText;
            scoreText.setFont(m_font);
            scoreText.setCharacterSize(16);
            scoreText.setColor(gf::Color::White);
            scoreText.setString(std::to_string(s.score));
            scoreText.setPosition({px + 5, py - 18});
            target.draw(scoreText, states);

            m_lastPacmanX = px;
            m_lastPacmanY = py;
            m_hasLastPacmanPos = true;
        } else { //Fantomes
            switch (ghostIndex) {
                case 0:
                    m_inkySprite.setPosition({px, py});
                    target.draw(m_inkySprite, states);
                    break;
                case 1:
                    m_clydeSprite.setPosition({px, py});
                    target.draw(m_clydeSprite, states);
                    break;
                case 2:
                    m_pinkySprite.setPosition({px, py});
                    target.draw(m_pinkySprite, states);
                    break;
                default:
                    m_blinkySprite.setPosition({px, py});
                    target.draw(m_blinkySprite, states);
                    break;
            }
            ++ghostIndex;
        }
    }
}
