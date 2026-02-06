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


void GameEntity::renderMap(gf::RenderTarget& target, const gf::RenderStates& states, const BoardCommon& map, float logicalTileSize, float mapOriginX, float mapOriginY) {
    for (unsigned int y = 0; y < map.height; ++y) {
        for (unsigned int x = 0; x < map.width; ++x) {
            const CaseCommon& cell = map.grid({ x, y });
            gf::RectangleShape tile({ logicalTileSize, logicalTileSize });
            tile.setPosition({ mapOriginX + x * logicalTileSize, mapOriginY + y * logicalTileSize });

            switch(cell.celltype) {
                case CellType::Wall:  tile.setColor(gf::Color::White); break;
                case CellType::Hut:   tile.setColor(gf::Color::Red); break;
                case CellType::Floor: tile.setColor(gf::Color::fromRgb(0.3f, 0.3f, 0.3f)); break;
                default:              tile.setColor(gf::Color::Black); break;
            }

            target.draw(tile, states);
        }
    }

    if (m_holeLinks.size() >= 2) {
        auto it = m_holeLinks.begin();
        gf::RectangleShape portalTile({ logicalTileSize, logicalTileSize });

        portalTile.setPosition({ mapOriginX + it->first.x * logicalTileSize, mapOriginY + it->first.y * logicalTileSize });
        portalTile.setColor(gf::Color::Magenta);
        target.draw(portalTile, states);

        portalTile.setPosition({ mapOriginX + it->second.x * logicalTileSize, mapOriginY + it->second.y * logicalTileSize });
        target.draw(portalTile, states);

        ++it;

        portalTile.setPosition({ mapOriginX + it->first.x * logicalTileSize, mapOriginY + it->first.y * logicalTileSize });
        portalTile.setColor(gf::Color::Blue);
        target.draw(portalTile, states);

        portalTile.setPosition({ mapOriginX + it->second.x * logicalTileSize, mapOriginY + it->second.y * logicalTileSize });
        target.draw(portalTile, states);
    }
}

void GameEntity::renderPacGommes(gf::RenderTarget& target, const gf::RenderStates& states, const std::vector<std::pair<Position, PacGommeType>>& pacgommes, float logicalTileSize, float mapOriginX, float mapOriginY) {
    float radius = logicalTileSize / 6.0f;

    for (const auto& [pos, type] : pacgommes) {
        gf::CircleShape pacGomme(radius);
        pacGomme.setOrigin({ radius, radius });
        pacGomme.setPosition({ mapOriginX + pos.x * logicalTileSize + logicalTileSize/2.f,
                               mapOriginY + pos.y * logicalTileSize + logicalTileSize/2.f });

        switch(type) {
            case PacGommeType::Basic: pacGomme.setColor(gf::Color::Yellow); break;
            case PacGommeType::Power: pacGomme.setColor(gf::Color::Green); break;
            default:                  pacGomme.setColor(gf::Color::White); break;
        }

        target.draw(pacGomme, states);
    }
}

void GameEntity::render(gf::RenderTarget& target, const gf::RenderStates& states) {
    const float LOGICAL_W = 1280.f;
    const float LOGICAL_H = 720.f;

    if (m_board.width == 0 || m_board.height == 0)
        return;

    float topMargin = 100.f; // espace réservé au timer en haut
    float padding = 20.f;   //marge autour de la map

    float logicalAvailableW = LOGICAL_W - 2*padding;
    float logicalAvailableH = LOGICAL_H - topMargin - padding; // hauteur dispo pour la map

    float logicalTileX = logicalAvailableW / float(m_board.width);
    float logicalTileY = logicalAvailableH / float(m_board.height);
    float logicalTileSize = std::min(logicalTileX, logicalTileY);

    float mapOriginX = padding + (logicalAvailableW - m_board.width*logicalTileSize)/2.f;
    float mapOriginY = topMargin + (logicalAvailableH - m_board.height*logicalTileSize)/2.f;

    float timerX = 20.f; // marge gauche
    float timerY = 50.f; // toujours visible en haut

    renderMap(target, states, m_board, logicalTileSize, mapOriginX, mapOriginY);
    renderPacGommes(target, states, m_pacgommes, logicalTileSize, mapOriginX, mapOriginY);

    gf::Text timer;
    timer.setFont(m_font);
    timer.setCharacterSize(24u);
    timer.setColor(gf::Color::White);
    if (m_timeLeftPre != 0)
        timer.setString("Temps avant le début de la partie : " + std::to_string(m_timeLeftPre));
    else
        timer.setString("Temps avant la fin de la partie : " + std::to_string(m_timeLeft));

    timer.setPosition({ timerX, timerY });
    target.draw(timer, states);

    float tilePx = logicalTileSize; // logique == pixels fixes

    if (m_inkyTexture.getSize().x>0) { float s = tilePx / float(m_inkyTexture.getSize().x); m_inkySprite.setScale({s,s}); }
    if (m_clydeTexture.getSize().x>0){ float s = tilePx / float(m_clydeTexture.getSize().x); m_clydeSprite.setScale({s,s}); }
    if (m_pinkyTexture.getSize().x>0){ float s = tilePx / float(m_pinkyTexture.getSize().x); m_pinkySprite.setScale({s,s}); }
    if (m_blinkyTexture.getSize().x>0){ float s = tilePx / float(m_blinkyTexture.getSize().x); m_blinkySprite.setScale({s,s}); }

    auto pacTexSize = m_pacmanRightTexture.getSize();
    if (pacTexSize.x>0) {
        float fw = float(pacTexSize.x)/4.f;
        float fh = float(pacTexSize.y);
        m_pacmanSprite.setScale({ tilePx/fw, tilePx/fh });
    }

    int ghostIndex = 0;
    for (const auto &s : m_states) {
        float px = mapOriginX + (s.x/50.f) * logicalTileSize;
        float py = mapOriginY + (s.y/50.f) * logicalTileSize;

        if (s.role == PlayerRole::PacMan) {
            if (m_hasLastPacmanPos) {
                if (px > m_lastPacmanX && m_pacmanDir!='R') { m_pacmanDir='R'; m_pacmanSprite.setAnimation(m_pacmanRightAnim); }
                else if (px < m_lastPacmanX && m_pacmanDir!='L') { m_pacmanDir='L'; m_pacmanSprite.setAnimation(m_pacmanLeftAnim); }
                else if (py > m_lastPacmanY && m_pacmanDir!='D') { m_pacmanDir='D'; m_pacmanSprite.setAnimation(m_pacmanDownAnim); }
                else if (py < m_lastPacmanY && m_pacmanDir!='U') { m_pacmanDir='U'; m_pacmanSprite.setAnimation(m_pacmanUpAnim); }
            }

            m_pacmanSprite.setPosition({ px, py });
            target.draw(m_pacmanSprite, states);

            gf::Text scoreText;
            scoreText.setFont(m_font);
            scoreText.setCharacterSize(16u);
            scoreText.setColor(gf::Color::White);
            scoreText.setString(std::to_string(s.score));
            scoreText.setPosition({ px+5.f, py-18.f });
            target.draw(scoreText, states);

            m_lastPacmanX = px;
            m_lastPacmanY = py;
            m_hasLastPacmanPos = true;
        } else { 
            if (ghostIndex==0) { m_inkySprite.setPosition({px,py}); target.draw(m_inkySprite, states); }
            else if (ghostIndex==1) { m_clydeSprite.setPosition({px,py}); target.draw(m_clydeSprite, states); }
            else if (ghostIndex==2) { m_pinkySprite.setPosition({px,py}); target.draw(m_pinkySprite, states); }
            else { m_blinkySprite.setPosition({px,py}); target.draw(m_blinkySprite, states); }
            ++ghostIndex;
        }
    }
}
