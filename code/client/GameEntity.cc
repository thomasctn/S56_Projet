#include "GameEntity.h"
#include "Renderer.h" // pour accéder à getRenderWindow()
#include <gf/Text.h>
#include <gf/Font.h>
#include <gf/Log.h>
#include <algorithm>

GameEntity::GameEntity(Renderer& renderer)
: m_renderer(renderer)
{
    //chargement des textures
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

    //pacman anim
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

void GameEntity::calculateMovement(const BoardCommon &map, float &tileSize, float &offsetX, float &offsetY) {
    //calcule d'après la view courante
    gf::RenderWindow& win = m_renderer.getRenderWindow();
    gf::View view = win.getView();
    gf::Vector2f viewSize = view.getSize();

    float topMargin = std::min(viewSize.y * 0.16f, 120.0f); //espace en haut pour texte
    float availW = viewSize.x;
    float availH = viewSize.y - topMargin;

    tileSize = std::min(availW / float(map.width), availH / float(map.height));
    offsetX = (viewSize.x - tileSize * float(map.width)) / 2.f + (view.getCenter().x - viewSize.x * 0.5f);
    offsetY = topMargin + (view.getCenter().y - viewSize.y * 0.5f);
}

void GameEntity::renderMap(const BoardCommon &map, float tileSize, float offsetX, float offsetY) {
    gf::RenderWindow& win = m_renderer.getRenderWindow();

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
            win.draw(tile);

            if (cell.celltype == CellType::Floor && cell.pacGomme) {
                gf::CircleShape pacGomme(tileSize / 6.0f);
                pacGomme.setOrigin({tileSize/12.0f, tileSize/12.0f});
                pacGomme.setPosition({
                    x * tileSize + offsetX + tileSize/2.0f,
                    y * tileSize + offsetY + tileSize/2.0f
                });
                pacGomme.setColor(gf::Color::Yellow);
                win.draw(pacGomme);
            }
        }
    }

    // header text (temps/titre) ? jsp ce qu'on mettra
    static gf::Font font("../common/fonts/arial.ttf");
    gf::Text title;
    title.setFont(font);
    title.setString("PacMan Game");
    title.setCharacterSize(24);
    title.setColor(gf::Color::White);

    //position relative à la view 
    gf::View view = win.getView();
    gf::Vector2f center = view.getCenter();
    gf::Vector2f size = view.getSize();
    float left = center.x -size.x * 0.5f;
    float top  = center.y -size.y * 0.5f;

    title.setPosition({ left + 10.f, top + 10.f });
    win.draw(title);
}

void GameEntity::renderPacGommes(const std::set<Position>& pacgommes, float tileSize, float offsetX, float offsetY) {
    gf::RenderWindow& win = m_renderer.getRenderWindow();
    for (const auto& pg : pacgommes) {
        gf::CircleShape pacGomme(tileSize / 6.0f);
        pacGomme.setOrigin({ tileSize / 12.0f, tileSize / 12.0f });
        pacGomme.setPosition({pg.x * tileSize + offsetX + tileSize / 2.0f,pg.y * tileSize + offsetY + tileSize / 2.0f});
        pacGomme.setColor(gf::Color::Yellow);
        win.draw(pacGomme);
    }
}

void GameEntity::render(const std::vector<PlayerData>& states, uint32_t myId, const BoardCommon& board, const std::set<Position>& pacgommes) {
    gf::RenderWindow& win = m_renderer.getRenderWindow();

    //clear via renderer helper
    m_renderer.clearWindow();

    float tileSize = 0.f;
    float offsetX = 0.f;
    float offsetY = 0.f;
    calculateMovement(board, tileSize, offsetX, offsetY);

    //map
    renderMap(board, tileSize, offsetX, offsetY);

    //pacgommes
    renderPacGommes(pacgommes, tileSize, offsetX, offsetY);

    //scale sprites pr tileSize
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

    for (const auto &s : states) {
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

            m_pacmanSprite.update(gf::seconds(0.008f));
            win.draw(m_pacmanSprite);

            //score 
            static gf::Font font("../common/fonts/arial.ttf");
            gf::Text scoreText;
            scoreText.setFont(font);
            scoreText.setCharacterSize(16);
            scoreText.setColor(gf::Color::White);
            scoreText.setString(std::to_string(s.score));
            scoreText.setPosition({px + 5, py - 18});
            win.draw(scoreText);

            m_lastPacmanX = px;
            m_lastPacmanY = py;
            m_hasLastPacmanPos = true;
        } else { //fantomes
            switch (ghostIndex) {
                case 0:
                    m_inkySprite.setPosition({px, py});
                    win.draw(m_inkySprite);
                    break;
                case 1:
                    m_clydeSprite.setPosition({px, py});
                    win.draw(m_clydeSprite);
                    break;
                case 2:
                    m_pinkySprite.setPosition({px, py});
                    win.draw(m_pinkySprite);
                    break;
                default:
                    m_blinkySprite.setPosition({px, py});
                    win.draw(m_blinkySprite);
                    break;
            }
            ++ghostIndex;
        }
    }

    //afficher
    win.display();
}
