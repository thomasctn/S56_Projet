#include "Board.h"

Board::Board(unsigned int w, unsigned int h)
: width(w), height(h), grid({w, h}) {

    // -- Generateur de labyrinthe ---
    generateMaze();
    //generateTestMaze();


}


Case &Board::getCase(unsigned int x, unsigned int y) {
    return grid({x, y});
}

const Case &Board::getCase(unsigned int x, unsigned int y) const {
    return grid({x, y});
}

bool Board::isInside(unsigned int x, unsigned int y) const {
    return x >= 0 && y >= 0 && x < width && y < height;
}

bool Board::isWalkable(unsigned int x, unsigned int y) const {
    if (x < 0 || y < 0 || x >= width || y >= height)
        return false;

    CellType type = grid({x, y}).getType();
    return type == CellType::Floor;
}

bool Board::isOccupied(unsigned int x, unsigned int y, uint32_t excludeId, const std::vector<Player> &players) const {
    for (auto &p : players)
    {
        if (p.id != excludeId && p.x == x && p.y == y)
            return true;
    }
    return false;
}



void Board::placeRandomPacGommes(unsigned int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distX(0, getWidth() - 1);
    std::uniform_int_distribution<> distY(0, getHeight() - 1);
    unsigned int placed = 0;
    while (placed < count) {
        unsigned int x = distX(gen);
        unsigned int y = distY(gen);
        Case& c = getCase(x, y);

        if (c.getType() == CellType::Floor && pacgommes.find(Position(x,y)) == pacgommes.end()) {
            pacgommes.insert(Position(x,y));
            ++placed;
        }
    }
}

void Board::generateMaze() {
    for (unsigned int y = 0; y < height; ++y){
        for (unsigned int x = 0; x < width; ++x)
        {
            grid({x,y}) = Case(CellType::Wall);
        }
    }
    generatePrimMaze();
    placeHut();
    connectHut();
    openCorners();
    //addLoops(0.15f);
    fillDeadEnds();

}


void Board::generatePrimMaze() {
    std::random_device rd;
    std::mt19937 gen(rd());

    auto inBounds = [&](unsigned int x, unsigned int y) {
        return x > 0 && y > 0 && x < width - 1 && y < height - 1;
    };

    std::vector<std::pair<unsigned int,unsigned int>> frontier;

    // Point de départ : coin supérieur gauche impair
    unsigned int sx = 1;
    unsigned int sy = 1;
    grid({sx,sy}) = Case(CellType::Floor);

    // Ajoute les murs voisins dans la frontier, sauf autour de la cabane
    auto addFrontier = [&](unsigned int x, unsigned int y)
    {
        if (inBounds(x, y) && grid({x,y}).getType() == CellType::Wall && !isHutWall(x, y))
        {
            frontier.emplace_back(x, y);
        }
    };

    addFrontier(sx + 2, sy);
    addFrontier(sx - 2, sy);
    addFrontier(sx, sy + 2);
    addFrontier(sx, sy - 2);

    while (!frontier.empty())
    {
        std::uniform_int_distribution<> dist(0, frontier.size() - 1);
        unsigned int idx = dist(gen);
        auto [x, y] = frontier[idx];
        frontier.erase(frontier.begin() + idx);

        if (grid({x,y}).getType() == CellType::Floor)
            continue;

        // Cherche les voisins déjà ouverts
        std::vector<std::pair<int,int>> neighbors;

        if (inBounds(x + 2, y) && grid({x + 2, y}).getType() == CellType::Floor)
            neighbors.emplace_back(x + 2, y);
        if (inBounds(x - 2, y) && grid({x - 2, y}).getType() == CellType::Floor)
            neighbors.emplace_back(x - 2, y);
        if (inBounds(x, y + 2) && grid({x, y + 2}).getType() == CellType::Floor)
            neighbors.emplace_back(x, y + 2);
        if (inBounds(x, y - 2) && grid({x, y - 2}).getType() == CellType::Floor)
            neighbors.emplace_back(x, y - 2);

        if (!neighbors.empty())
        {
            std::uniform_int_distribution<> ndist(0, neighbors.size() - 1);
            auto [nx, ny] = neighbors[ndist(gen)];

            // Casse le mur entre les deux, sauf si c'est un mur de cabane
            unsigned int wallX = (x + nx) / 2;
            unsigned int wallY = (y + ny) / 2;

            if (!isHutWall(wallX, wallY))
                grid({wallX, wallY}) = Case(CellType::Floor);

            if (!isHutWall(x, y))
                grid({x,y}) = Case(CellType::Floor);

            // Ajouter les nouveaux murs à frontier, sauf cabane
            addFrontier(x + 2, y);
            addFrontier(x - 2, y);
            addFrontier(x, y + 2);
            addFrontier(x, y - 2);
        }
    }
}


void Board::placeHut() {
    unsigned int cx = width / 2;
    unsigned int cy = height / 2;

    // Cabane
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            grid({static_cast<unsigned int>(cx + dx),static_cast<unsigned int>(cy + dy)}) = Case(CellType::Hut);
        }
    }

    // Mur autour
    for (int dx = -2; dx <= 2; ++dx)
    {
        if (cx + dx > 0 && cx + dx < (int)width)
        {
            if (cy-2 > 0) grid({static_cast<unsigned int>(cx+dx),static_cast<unsigned int>(cy-2)}) = Case(CellType::Wall);
            if (cy+2 < (int)height) grid({static_cast<unsigned int>(cx+dx),static_cast<unsigned int>(cy+2)}) = Case(CellType::Wall);
        }
    }
    for (int dy = -1; dy <= 1; ++dy)
    {
        if (cx-2 > 0) grid({static_cast<unsigned int>(cx-2), static_cast<unsigned int>(cy+dy)}) = Case(CellType::Wall);
        if (cx+2 < (int)width) grid({static_cast<unsigned int>(cx+2), static_cast<unsigned int>(cy+dy)}) = Case(CellType::Wall);
    }
}


void Board::connectHut() {
    unsigned int cx = width / 2;
    unsigned int cy = height / 2;

    struct Dir { int dx, dy; };
    std::vector<Dir> dirs = {
        { 0, -1 }, // haut
        { 0,  1 }, // bas
        { -1, 0 }, // gauche
        { 1,  0 }  // droite
    };

    for (auto d : dirs)
    {
        // Mur juste à l'extérieur de la cabane
        int wx = cx + d.dx * 2;
        int wy = cy + d.dy * 2;

        if (wx <= 0 || wy <= 0 || wx >= (int)width - 1 || wy >= (int)height - 1)
            continue;

        // Vérifie si la cellule au-delà du mur est déjà Floor
        int nx = cx + d.dx * 3;
        int ny = cy + d.dy * 3;
        if (!isInside(nx, ny))
            continue;

        if (grid({static_cast<unsigned int>(nx), static_cast<unsigned int>(ny)}).getType() == CellType::Floor)
        {
            // Casse le mur entre la cabane et le labyrinthe
            grid({static_cast<unsigned int>(wx), static_cast<unsigned int>(wy)}) = Case(CellType::Floor);
        }
    }

    // Si aucune direction n'était connectée, on force une sortie aléatoire
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(dirs.begin(), dirs.end(), gen);

    for (auto d : dirs)
    {
        int wx = cx + d.dx * 2;
        int wy = cy + d.dy * 2;
        int nx = cx + d.dx * 3;
        int ny = cy + d.dy * 3;

        if (!isInside(wx, wy) || !isInside(nx, ny))
            continue;

        grid({static_cast<unsigned int>(wx), static_cast<unsigned int>(wy)}) = Case(CellType::Floor);
        grid({static_cast<unsigned int>(nx), static_cast<unsigned int>(ny)}) = Case(CellType::Floor);
        return;
    }
}



void Board::openCorners() {
    std::vector<std::pair<unsigned int,unsigned int>> corners = {
        {1,1},
        {width - 2, 1},
        {1, height - 2},
        {width - 2, height - 2}
    };

    for (auto [x, y] : corners)
    {
        grid({x,y}) = Case(CellType::Floor);

        // Connecte vers l'intérieur
        if (grid({static_cast<unsigned int>(x + (x == 1 ? 1 : -1)), y}).getType() == CellType::Wall)
            grid({static_cast<unsigned int>(x + (x == 1 ? 1 : -1)), y}) = Case(CellType::Floor);

        if (grid({x, static_cast<unsigned int>(y + (y == 1 ? 1 : -1))}).getType() == CellType::Wall)
            grid({x, static_cast<unsigned int>(y + (y == 1 ? 1 : -1))}) = Case(CellType::Floor);
    }
}

void Board::addLoops(float probability) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    unsigned int cx = width / 2;
    unsigned int cy = height / 2;

    for (unsigned int y = 1; y < height - 1; ++y)
    {
        for (unsigned int x = 1; x < width - 1; ++x)
        {
            if (grid({x,y}).getType() != CellType::Wall)
                continue;

            // Ne touche pas à la cabane (zone 3x3 + marge)
            if (x >= cx - 2 && x <= cx + 2 &&
                y >= cy - 2 && y <= cy + 2)
                continue;

            // Cas horizontal : sol mur sol
            bool h =
                grid({x - 1, y}).getType() == CellType::Floor &&
                grid({x + 1, y}).getType() == CellType::Floor;

            // Cas vertical : sol mur sol
            bool v =
                grid({x, y - 1}).getType() == CellType::Floor &&
                grid({x, y + 1}).getType() == CellType::Floor;

            if ((h || v) && chance(gen) < probability)
            {
                grid({x,y}) = Case(CellType::Floor);
            }
        }
    }
}

void Board::fillDeadEnds() {
    bool changed = true;

    while (changed)
    {
        changed = false;

        for (unsigned int y = 1; y < height - 1; ++y) // <- ne touche pas les bords
        {
            for (unsigned int x = 1; x < width - 1; ++x) // <- ne touche pas les bords
            {
                if (grid({x,y}).getType() != CellType::Floor)
                    continue;

                // Ne touche pas la cabane
                unsigned int cx = width / 2;
                unsigned int cy = height / 2;
                if (x >= cx - 1 && x <= cx + 1 && y >= cy - 1 && y <= cy + 1)
                    continue;

                // Compter les cases Floor autour
                unsigned int floorCount = 0;
                if (grid({x + 1, y}).getType() == CellType::Floor) floorCount++;
                if (grid({x - 1, y}).getType() == CellType::Floor) floorCount++;
                if (grid({x, y + 1}).getType() == CellType::Floor) floorCount++;
                if (grid({x, y - 1}).getType() == CellType::Floor) floorCount++;

                // Si c'est un cul-de-sac
                if (floorCount == 1)
                {
                    // Chercher un mur adjacent à casser (hors bordure)
                    std::vector<std::pair<unsigned int,unsigned int>> candidates;
                    if (x + 1 < width - 1 && grid({x + 1, y}).getType() == CellType::Wall) candidates.push_back({x+1, y});
                    if (x - 1 > 0       && grid({x - 1, y}).getType() == CellType::Wall) candidates.push_back({x-1, y});
                    if (y + 1 < height - 1 && grid({x, y + 1}).getType() == CellType::Wall) candidates.push_back({x, y+1});
                    if (y - 1 > 0       && grid({x, y - 1}).getType() == CellType::Wall) candidates.push_back({x, y-1});

                    if (!candidates.empty())
                    {
                        // Prend un mur au hasard et le transforme en Floor
                        std::random_device rd;
                        std::mt19937 gen(rd());
                        std::uniform_int_distribution<> dist(0, candidates.size() - 1);
                        auto [mx, my] = candidates[dist(gen)];

                        grid({mx, my}) = Case(CellType::Floor);
                        changed = true;
                    }
                }
            }
        }
    }
}


bool Board::isHutWall(unsigned int x, unsigned int y) {
    int cx = width / 2;
    int cy = height / 2;

    // Seule la cabane 3x3 est protégée
    return (x >= cx - 1 && x <= cx + 1 &&
            y >= cy - 1 && y <= cy + 1);
}


void Board::generateTestMaze(){
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            // Les murs autour de la grille
            if (y == 0 || y == height - 1 || x == 0 || x == width - 1)
            {
                grid({x,y}) = Case(CellType::Wall);
            }
            // La cabane centrale 3x3
            else if ((x >= width/2 - 1 && x <= width/2 + 1) &&
                     (y >= height/2 - 1 && y <= height/2 + 1))
            {
                grid({x,y}) = Case(CellType::Hut);
            }
            // Le reste = sol
            else
            {
                grid({x,y}) = Case(CellType::Floor);
            }
        }
    }
}






/******PRINT*****/
void Board::print() const {
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            switch (grid({x, y}).getType())
            {
            case CellType::Floor:
                std::cout << ".";
                break;
            case CellType::Wall:
                std::cout << "#";
                break;
            case CellType::Hut:
                std::cout << "H";
                break;
            }
        }
        std::cout << "\n";
    }
}

void Board::printWithPlayers(const std::vector<Player> &players) const {
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {

            bool playerHere = false;

            for (const auto &p : players)
            {
                if (p.x == x && p.y == y)
                {
                    std::cout << p.id;
                    playerHere = true;
                    break;
                }
            }

            if (!playerHere)
            {
                switch (grid({x, y}).getType())
                {
                case CellType::Floor:
                    std::cout << ".";
                    break;
                case CellType::Wall:
                    std::cout << "#";
                    break;
                case CellType::Hut:
                    std::cout << "H";
                    break;
                }
            }
        }
        std::cout << "\n";
    }
}

bool Board::hasPacgomme(unsigned int x, unsigned int y)
{
    return pacgommes.find(Position(x,y)) != pacgommes.end();
}

bool Board::removePacgomme(unsigned int x, unsigned int y)
{
    if(!hasPacgomme(x,y))
        return false;
    pacgommes.erase(Position(x,y));
    return true;
}

BoardCommon Board::toCommonData()
{
    BoardCommon bc = BoardCommon(width,height);
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            bc.grid({x, y}) = grid({x, y}).toCommonData();
        }
    }
    return bc;
}
