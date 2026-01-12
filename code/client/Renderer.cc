#include "Renderer.h"

#include <gf/Log.h>
#include <gf/Text.h>
#include <gf/Font.h>

Renderer::Renderer() : main_window("GF Sync Boxes", {800,600}), rendered_window(main_window){
    //POUR LE SPRITE 
    m_inkyTexture = gf::Texture("../client/assets/ghosts/inky.png"); 
    m_inkySprite.setTexture(m_inkyTexture);
    m_inkySprite.setOrigin({0.f, 0.f}); //en haut a gauche
    m_inkySprite.setScale({2.f, 2.f}); //sa taille (temp ce sera recalculé norùalement)
    
    m_clydeTexture = gf::Texture("../client/assets/ghosts/clyde.png"); 
    m_clydeSprite.setTexture(m_clydeTexture);
    m_clydeSprite.setOrigin({0.f, 0.f}); //en haut a gauche
    m_clydeSprite.setScale({2.f, 2.f}); //sa taille (temp ce sera recalculé norùalement)

    m_pinkyTexture = gf::Texture("../client/assets/ghosts/pinky.png"); 
    m_pinkySprite.setTexture(m_pinkyTexture);
    m_pinkySprite.setOrigin({0.f, 0.f}); //en haut a gauche
    m_pinkySprite.setScale({2.f, 2.f}); //sa taille (temp ce sera recalculé norùalement)

    m_blinkyTexture = gf::Texture("../client/assets/ghosts/blinky.png"); 
    m_blinkySprite.setTexture(m_blinkyTexture);
    m_blinkySprite.setOrigin({0.f, 0.f}); //en haut a gauche
    m_blinkySprite.setScale({2.f, 2.f}); //sa taille (temp ce sera recalculé norùalement)

    /*tests pacman animé
    m_pacmanTexture = gf::Texture("../client/assets/pacman/right_pacman_sheet.png");
    m_pacmanAnimation.addTileset(m_pacmanTexture, {4,1}, gf::seconds(0.1f), 4); 
    m_pacmanAnimation.setLoop(true);*/

    // Lier l'animation au sprite
    //m_pacmanSprite.setAnimation(m_pacmanAnimation);


    m_pacmanRightTexture = gf::Texture("../client/assets/pacman/right_pacman_sheet.png");
    m_pacmanLeftTexture = gf::Texture("../client/assets/pacman/left_pacman_sheet.png");
    m_pacmanUpTexture = gf::Texture("../client/assets/pacman/up_pacman_sheet.png");
    m_pacmanDownTexture = gf::Texture("../client/assets/pacman/down_pacman_sheet.png");
    m_pacmanRightAnim.addTileset(m_pacmanRightTexture,{4, 1},gf::seconds(0.1f), 4);
    m_pacmanLeftAnim.addTileset(m_pacmanLeftTexture,{4, 1},gf::seconds(0.1f), 4);
    m_pacmanUpAnim.addTileset(m_pacmanUpTexture,{4, 1},gf::seconds(0.1f), 4);
    m_pacmanDownAnim.addTileset(m_pacmanDownTexture,{4, 1},gf::seconds(0.1f), 4);
    m_pacmanRightAnim.setLoop(true);
    m_pacmanLeftAnim.setLoop(true);
    m_pacmanUpAnim.setLoop(true);
    m_pacmanDownAnim.setLoop(true);





    m_pacmanSprite.setAnimation(m_pacmanRightAnim); // direction par défaut
    m_pacmanDir = 'R';

    // Centrer ou placer l'origine si nécessaire
    m_pacmanSprite.setOrigin({0.f, 0.f});

    
    
    
    m_view.setSize({m_worldSize, m_worldSize});
    m_view.setCenter({m_worldSize/ 2.f, m_worldSize / 2.f});
    rendered_window.setView(m_view);
}

gf::Color4f Renderer::colorFromId(uint32_t id) {
    float r = float((id * 50) % 256) / 255.0f;
    float g = float((id * 80) % 256) / 255.0f;
    float b = float((id * 110) % 256) / 255.0f;
    return gf::Color4f(r, g, b, 1.0f);
}




void Renderer::render(const std::vector<PlayerData>& states, uint32_t myId, const BoardCommon map,const std::set<Position>& pacgommes){
    rendered_window.clear(gf::Color::Black);
    renderMap(states,map);
    
    //AVANT : LES PXIELS JOUEURS
    /*for (auto& s : states) {
        gf::RectangleShape box({50.0f, 50.0f});
        box.setPosition({s.x, s.y});
        gf::Color4f c = (s.id == myId) ? gf::Color4f(1, 0, 0, 1) : colorFromId(s.id);
        box.setColor(c);
        rendered_window.draw(box);
    }*/

    //apres, test sprit:
    float tileSize, offsetX, offsetY;
    calculateMovement(m_worldSize, map, tileSize, offsetX, offsetY);
    renderPacGommes(pacgommes,tileSize,offsetX,offsetY);

    //TEMPORAIRE calculer correctement la taille du sprite!
    auto texSize=m_inkyTexture.getSize();
    if (texSize.x>0 && texSize.y>0){
        float scaleX= tileSize/float(texSize.x);
        float scaleY= tileSize/float(texSize.y);
        m_inkySprite.setScale({scaleX, scaleY });
        m_clydeSprite.setScale({scaleX, scaleY });
        m_pinkySprite.setScale({scaleX, scaleY});
        m_blinkySprite.setScale({scaleX, scaleY});

    }

    int ghostIndex = 0;

    for (const auto &s : states) {
        float px = s.x / 50.0f * tileSize + offsetX;
        float py = s.y / 50.0f * tileSize + offsetY;
        if(s.role == PlayerRole::PacMan){ //joueur 1 est pacman. a remplacer par es roles

            if (m_hasLastPacmanPos){                
                if(px > m_lastPacmanX){ //droite
                    if(m_pacmanDir != 'R'){
                        m_pacmanDir = 'R';
                        m_pacmanSprite.setAnimation(m_pacmanRightAnim);
                    }
                }
                else if(px < m_lastPacmanX){ //gauche
                    if(m_pacmanDir != 'L'){
                        m_pacmanDir = 'L';
                        m_pacmanSprite.setAnimation(m_pacmanLeftAnim);
                    }
                }

                else if(py > m_lastPacmanY){ //bas
                    if(m_pacmanDir != 'D'){
                        m_pacmanDir = 'D';
                        m_pacmanSprite.setAnimation(m_pacmanDownAnim);
                    }
                }
                else if(py < m_lastPacmanY){//haut
                    if(m_pacmanDir != 'U'){
                        m_pacmanDir = 'U';
                        m_pacmanSprite.setAnimation(m_pacmanUpAnim);
                    }
                }
            }



            m_pacmanSprite.setPosition({ px, py });

            auto texSize = m_pacmanRightTexture.getSize();
            float frameWidth  = texSize.x/4.0f;
            float frameHeight = texSize.y;

            float scaleX= tileSize/frameWidth;
            float scaleY= tileSize/frameHeight;

            m_pacmanSprite.setScale({scaleX, scaleY});

            m_pacmanSprite.update(gf::seconds(0.008f));  //maj de l'anim
            rendered_window.draw(m_pacmanSprite);

            //score
            static gf::Font font("../common/fonts/arial.ttf");
            gf::Text scoreText;
            scoreText.setFont(font);
            scoreText.setCharacterSize(16);
            scoreText.setColor(gf::Color::White);
            scoreText.setString(std::to_string(s.score));
            scoreText.setPosition({px + 5 , py - 18});
            rendered_window.draw(scoreText);

            m_lastPacmanX = px;
            m_lastPacmanY = py;
            m_hasLastPacmanPos = true;

        }
        else if(s.role == PlayerRole::Ghost) {


            if(ghostIndex == 0) {
                m_inkySprite.setPosition({px, py }); 
                rendered_window.draw(m_inkySprite);
            }
            else if(ghostIndex == 1) {
                m_clydeSprite.setPosition({px, py }); 
                rendered_window.draw(m_clydeSprite);
            }
            else if(ghostIndex == 2) {
                m_pinkySprite.setPosition({px, py }); 
                rendered_window.draw(m_pinkySprite);
            }
            else{
                m_blinkySprite.setPosition({px, py }); 
                rendered_window.draw(m_blinkySprite);
            }

            ghostIndex++;
        }

    }

    /*Pour clyde
    m_clydeSprite.setPosition({ px, py });
            rendered_window.draw(m_clydeSprite);*/


    /*for (const auto& s : states) {
        m_inkySprite.setPosition({s.x, s.y});
        rendered_window.draw(m_inkySprite);
    }*/

  
    
    rendered_window.display();


}



void Renderer::renderMap(const std::vector<PlayerData>& states, const BoardCommon map){
    BoardCommon mapPerso = map;
    //sans le responsive:
    float tileSize = std::min(m_worldSize / mapPerso.width, m_worldSize / mapPerso.height); //a remplacer par ma fonction utilistaire
    float offsetX = (m_worldSize - tileSize * mapPerso.width) / 2.f;
    float offsetY = (m_worldSize - tileSize * mapPerso.height) / 2.f;


    bool isFloor = false;
    for (unsigned int y = 0; y < mapPerso.height; ++y) {
                for (unsigned int x = 0; x < mapPerso.width; ++x) {
                    const CaseCommon& cell = mapPerso.grid({ x, y });

                    gf::RectangleShape tile({tileSize, tileSize});
                    tile.setPosition({x * tileSize + offsetX, y * tileSize + offsetY});

                    switch (cell.celltype) {
                        case CellType::Wall: tile.setColor(gf::Color::White); break;
                        case CellType::Hut:  tile.setColor(gf::Color::Red); break;
                        case CellType::Floor: isFloor = true; tile.setColor(gf::Color::fromRgb(0.3f, 0.3f, 0.3f)); break;
                        default: continue;
                    }
                    rendered_window.draw(tile);

                    //affichage pac gomme
                    if (isFloor && cell.pacGomme) {
                        gf::CircleShape pacGomme(tileSize / 6.0f);
                        pacGomme.setOrigin({tileSize/12.0f, tileSize/12.0f });

                        pacGomme.setPosition({
                            x * tileSize + offsetX + tileSize/2.0f,
                            y * tileSize + offsetY + tileSize/2.0f
                        });

                        pacGomme.setColor(gf::Color::Yellow);
                        rendered_window.draw(pacGomme);
                    }
                }
            }
}

void Renderer::renderPacGommes(const std::set<Position>& pacgommes,float tileSize,float offsetX,float offsetY){

    for (const auto& pg : pacgommes) {
        gf::CircleShape pacGomme(tileSize / 6.0f);

        pacGomme.setOrigin({
            tileSize / 12.0f,
            tileSize / 12.0f
        });

        pacGomme.setPosition({
            pg.x * tileSize + offsetX + tileSize / 2.0f,
            pg.y * tileSize + offsetY + tileSize / 2.0f
        });

        pacGomme.setColor(gf::Color::Yellow);

        rendered_window.draw(pacGomme);
    }
}


void Renderer::handleResize(unsigned int winW, unsigned int winH)
{
    float windowRatio = float(winW) / float(winH);
    float worldRatio  = 1.0f; // monde carré

    if (windowRatio > worldRatio) {
        m_view.setSize({m_worldSize* windowRatio, m_worldSize}); // fenêtre trop large
    } else {
        m_view.setSize({m_worldSize, m_worldSize/windowRatio}); // fenêtre trop haute
    }

    m_view.setCenter({m_worldSize / 2.f, m_worldSize / 2.f});
    rendered_window.setView(m_view);
}


void Renderer::calculateMovement(float worldSize, const BoardCommon &map, float &tileSize, float &offsetX, float &offsetY) {
    tileSize = std::min(worldSize / float(map.width), worldSize / float(map.height));

    offsetX = (worldSize - tileSize *float(map.width)) / 2.f;
    offsetY = (worldSize - tileSize *float(map.height)) / 2.f;
}

void Renderer::renderWelcome() {
    rendered_window.clear(gf::Color::Black);

    static gf::Font font("../common/fonts/arial.ttf");

    //Titre PACMAN 
    gf::Text title;
    title.setFont(font);
    title.setCharacterSize(40);
    title.setColor(gf::Color::White);
    title.setString("PACMAN");
    title.setPosition({20.f, 20.f});
    rendered_window.draw(title);

    //Bouton rectangle
    int bx= 20;
    int by= 80;
    int bw= 200;
    int bh= 80;

    gf::RectangleShape button({float(bw), float(bh)});
    button.setPosition({float(bx), float(by)});
    button.setColor(gf::Color::fromRgb(0.2f, 0.2f, 0.8f));
    rendered_window.draw(button);

    //Texte du bouton centré a peu pres dans le rectangle
    gf::Text buttonText;
    buttonText.setFont(font);
    buttonText.setCharacterSize(24);
    buttonText.setColor(gf::Color::White);
    buttonText.setString("ENTRER");
    buttonText.setPosition({
        float(bx)+ 20.f, //decal horizontal
        float(by)+ 25.f  //decal vertical
    });

    rendered_window.draw(buttonText);

    rendered_window.display();
}

void Renderer::renderLobby(int connectedPlayers, int roomSize, bool amReady) {
    rendered_window.clear(gf::Color::Black);

    static gf::Font font("../common/fonts/arial.ttf");

    //uilisation monde
    float worldSize = m_worldSize; //monde carré
    float margin = 20.f;

    // positions boutons
    m_btnSize = gf::v1::Vector2f{40.f, 40.f};
    m_minusBtnPos = gf::v1::Vector2f{margin + 140.f, 100.f};
    m_plusBtnPos  = gf::v1::Vector2f{m_minusBtnPos.x + m_btnSize.x + 60.f, 100.f};

    m_readyBtnSize = gf::v1::Vector2f{worldSize*0.18f, worldSize*0.08f};
    m_readyBtnPos  = gf::v1::Vector2f{margin, 200.f};

    //textes
    gf::Text title;
    title.setFont(font);
    title.setCharacterSize(28);
    title.setColor(gf::Color::White);
    title.setString("En attente de joueurs...");
    title.setPosition({margin, 20.f});
    rendered_window.draw(title);

    gf::Text countText;
    countText.setFont(font);
    countText.setCharacterSize(20);
    countText.setColor(gf::Color::White);
    countText.setString(std::to_string(connectedPlayers) + " / " + std::to_string(roomSize));
    countText.setPosition({margin, 60.f});
    rendered_window.draw(countText);

    gf::Text roomLabel;
    roomLabel.setFont(font);
    roomLabel.setCharacterSize(20);
    roomLabel.setColor(gf::Color::White);
    roomLabel.setString("Joueurs max :");
    roomLabel.setPosition({margin,100.f});
    rendered_window.draw(roomLabel);

    //boutons augmenter et diminuer
    gf::RectangleShape minusBtn(m_btnSize);
    minusBtn.setPosition(m_minusBtnPos);
    minusBtn.setColor(gf::Color::fromRgb(0.2f, 0.2f, 0.8f)); // gris
    rendered_window.draw(minusBtn);

    gf::Text minusText;
    minusText.setFont(font);
    minusText.setCharacterSize(24);
    minusText.setColor(gf::Color::White);
    minusText.setString("-");
    minusText.setPosition({m_minusBtnPos.x + 10.f, m_minusBtnPos.y + 5.f});
    rendered_window.draw(minusText);

    gf::RectangleShape plusBtn(m_btnSize);
    plusBtn.setPosition(m_plusBtnPos);
    plusBtn.setColor(gf::Color::fromRgb(0.2f, 0.2f, 0.8f));
    rendered_window.draw(plusBtn);

    gf::Text plusText;
    plusText.setFont(font);
    plusText.setCharacterSize(24);
    plusText.setColor(gf::Color::White);
    plusText.setString("+");
    plusText.setPosition({m_plusBtnPos.x + 10.f, m_plusBtnPos.y + 5.f});
    rendered_window.draw(plusText);

    // valeur
    gf::Text valueText;
    valueText.setFont(font);
    valueText.setCharacterSize(20);
    valueText.setColor(gf::Color::White);
    valueText.setString(std::to_string(roomSize));
    valueText.setPosition({m_minusBtnPos.x + m_btnSize.x + 10.f, m_minusBtnPos.y + 8.f});
    rendered_window.draw(valueText);

    // pret boutton
    gf::RectangleShape readyBtn(m_readyBtnSize);
    readyBtn.setPosition(m_readyBtnPos);
    if (amReady){
        readyBtn.setColor(gf::Color::fromRgb(0.7f, 0.2f, 0.2f)); //environ rouge
    } else {
        readyBtn.setColor(gf::Color::fromRgb(0.2f, 0.7f, 0.2f)); //environ vert
    }
        rendered_window.draw(readyBtn);

    gf::Text readyText;
    readyText.setFont(font);
    readyText.setCharacterSize(int(m_readyBtnSize.y*0.5f));
    readyText.setColor(gf::Color::White);
    readyText.setString(amReady ? "PLUS PRÊT?" : "PRÊT");
    readyText.setPosition({m_readyBtnPos.x + 10.f, m_readyBtnPos.y + m_readyBtnSize.y*0.2f});
    rendered_window.draw(readyText);

    // pret
    gf::Text readyState;
    readyState.setFont(font);
    readyState.setCharacterSize(20);
    readyState.setColor(gf::Color::White);
    readyState.setString(amReady ? "Vous êtes : PRÊT" : "Vous êtes : PAS PRÊT");
    readyState.setPosition({margin, 160.f});
    rendered_window.draw(readyState);

    rendered_window.display();
}


