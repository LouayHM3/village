#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <conio.h>
#include <cstdlib>
#include <windows.h>
#include <ctime>
using namespace std;

// Dimensions de la grille
const int gridWidth = 20;
const int gridHeight = 10;

// ==== Nettoyage du curseur ====
void moveCursorTopLeft() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hConsole, pos);
}

// ==== Classe ResourceGenerator et dérivés ====
class ResourceGenerator {
protected:
    int Value;

public:
    int x, y;
    ResourceGenerator(int x, int y, int value = 0) : x(x), y(y), Value(value) {}
    virtual ~ResourceGenerator() {}  // Destructeur virtuel
    virtual std::string getRepr() const = 0;
    virtual std::string getType() const = 0;
    virtual int getValue() const { return Value; }
    bool isAt(int px, int py) const { return x == px && y == py; }
};

class GoldMine : public ResourceGenerator {
public:
    GoldMine(int x, int y, int value = 40) : ResourceGenerator(x, y, value) {}
    std::string getRepr() const override { return "🪙"; }
    std::string getType() const override { return "gold"; }
};

class ElixirCollector : public ResourceGenerator {
public:
    ElixirCollector(int x, int y, int value = 60) : ResourceGenerator(x, y, value) {}
    std::string getRepr() const override { return "🔮"; }
    std::string getType() const override { return "elixir"; }
};

// ==== Classe de base Entity ====
class Entity {
protected:
    int x, y;
    std::string repr;

public:
    Entity(int x = 0, int y = 0, const std::string& repr = "*") : x(x), y(y), repr(repr) {}
    virtual ~Entity() = default;

    void setPosition(int newX, int newY) {
        x = (newX >= 0 && newX < gridWidth) ? newX : x;
        y = (newY >= 0 && newY < gridHeight) ? newY : y;
    }

    std::string getRepr() const { return repr; }
    int getX() const { return x; }
    int getY() const { return y; }
    bool isAt(int px, int py) const { return x == px && y == py; }
};

// ==== Classe Player ====
class Player : public Entity {
public:
    int gold = 0;
    int elixir = 0;
    bool hasWalls = false;

    Player(int x = 0, int y = 0) : Entity(x, y, "🛡️") {}

    void move(char direction, int w, int h) {
        switch (direction) {
            case 'w': case 'z': setPosition(x, y - 1); break;
            case 's': setPosition(x, y + 1); break;
            case 'a': case 'q': setPosition(x - 1, y); break;
            case 'd': setPosition(x + 1, y); break;
        }
    }

    void collectFrom(ResourceGenerator* rg) {
        if (rg->getType() == "gold") {
            gold += rg->getValue();
        } else if (rg->getType() == "elixir") {
            elixir += rg->getValue();
        }
    }

    void buildWalls() {
        if (gold >= 100) {
            hasWalls = true;
            gold -= 100;
        }
    }

    void strengthenWalls() {
        if (elixir >= 50) {
            elixir -= 50;
        }
    }
};

// ==== Classe Enemy ====
class Enemy : public Entity {
public:
    Enemy(int x = 0, int y = 0) : Entity(x, y, "👾") {}

    void moveRandom() {
        int randDirection = rand() % 4;
        switch (randDirection) {
            case 0: setPosition(x, y - 1); break;
            case 1: setPosition(x, y + 1); break;
            case 2: setPosition(x - 1, y); break;
            case 3: setPosition(x + 1, y); break;
        }
    }
};

// ==== Classe Building ====
class Building : public Entity {
public:
    Building(int x, int y, const std::string& repr) : Entity(x, y, repr) {}

    static Building TownHall(int x, int y) {
        return Building(x, y, "🏛️");
    }

    static Building Wall(int x, int y) {
        return Building(x, y, "🧱");
    }
};

// ==== Affichage de la grille ====
void drawGrid(const Player& player, const Enemy& enemy, const std::vector<ResourceGenerator*>& resources, const std::vector<Building>& buildings) {
    vector<vector<string>> grid(gridHeight, vector<string>(gridWidth, " "));

    for (auto r : resources) {
        grid[r->y][r->x] = r->getRepr();
    }

    for (auto& b : buildings) {
        grid[b.getY()][b.getX()] = b.getRepr();
    }

    grid[player.getY()][player.getX()] = player.getRepr();
    grid[enemy.getY()][enemy.getX()] = enemy.getRepr();

    moveCursorTopLeft();
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            cout << grid[y][x] << " ";
        }
        cout << endl;
    }

    cout << "\n🎒 Ressources — Or: " << player.gold << " | Élixir: " << player.elixir << "\n";
    cout << "Utilise ZQSD (ou WASD) pour bouger, 'x' pour quitter, espace pour construire un mur.\n";
}

// ==== Collection de ressources ====
void checkForCollection(Player& player, std::vector<ResourceGenerator*>& resources) {
    for (auto it = resources.begin(); it != resources.end(); ++it) {
        if ((*it)->isAt(player.getX(), player.getY())) {
            player.collectFrom(*it);
           