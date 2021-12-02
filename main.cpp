
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || \
    defined(__MINGW32__) || defined(__BORLANDC__)
#define OS_WIN
#endif

// M_PI not defined on windows?
#ifdef OS_WIN
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>

#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "util.h"

enum MenuState {
    EXIT = -1,

    ROOT = 0,
    ABOUT,
    PAUSED,
    SETTINGS,
    GAME,
};

// size of each grid spot
constexpr float GRID_SIZEF = 100.f;
constexpr int GRID_SIZE = static_cast<int>(GRID_SIZEF);
// number of grid items in the world
constexpr int WORLD_GRID_SIZE = 10;
// TODO - can we lower this? it feels too long
constexpr float PLAYER_TILE_REACH = 0.5;
constexpr float P_MOVE_DIST = 0.05;

struct Tile : public sf::Drawable {
    sf::RectangleShape shape;

    Tile() {
        shape.setSize(vec2f{GRID_SIZEF, GRID_SIZEF});
        shape.setFillColor(sf::Color::White);
        shape.setOutlineThickness(1.0f);
        shape.setOutlineColor(sf::Color::Black);
    }
    void setPosition(vec2f position) { shape.setPosition(position); }
    vec2f getPosition() { return shape.getPosition(); }

    void update(sf::Time dt) {}

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(shape);
    }
};
std::vector<std::vector<Tile>> tilemap;

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
struct Item : public sf::Drawable {
    std::string name;
    double price;
    vec2f tilePosition;
    sf::Color color;

    Item(std::string n, double p, sf::Color c) : name(n), price(p), color(c) {}
    ~Item() {}
    Item(const Item &i)
        : name(i.name),
          price(i.price),
          tilePosition(i.tilePosition),
          color(i.color) {}
    Item &operator=(const Item &i) {
        if (this == &i) return *this;
        this->name = i.name;
        this->price = i.price;
        this->tilePosition = i.tilePosition;
        this->color = i.color;
        return *this;
    }

    Item(std::string n, double p) : name(n), price(p) {}
    bool operator<(const Item &i) const { return this->name < i.name; }

    void update(sf::Time dt) {}

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        sf::RectangleShape s =
            sf::RectangleShape(vec2f{GRID_SIZEF / 4, GRID_SIZEF / 4});
        s.setPosition(
            vec2f{tilePosition.x * GRID_SIZEF, tilePosition.y * GRID_SIZEF});
        s.setFillColor(color);
        target.draw(s);
    }
};

const std::vector<Item> all_items = std::vector<Item>{
    Item("apple", 1.0, sf::Color(196, 51, 64)),
};

struct Desire {
    Item item;
    int amount;
    Desire(Item i, int a) : item(i), amount(a) {}
    ~Desire() {}
    Desire(const Desire &d) : item(d.item), amount(d.amount) {}

    Desire &operator=(const Desire &d) {
        if (this == &d) return *this;
        this->item = d.item;
        this->amount = d.amount;
        return *this;
    }
};
typedef std::vector<Desire> Desires;

struct Shelf : public sf::Drawable {
    Desires contents;
    vec2f tilePosition;

    sf::RectangleShape shape;

    Shelf() {
        shape = sf::RectangleShape(vec2f{GRID_SIZEF, GRID_SIZEF});
        shape.setFillColor(sf::Color::Black);
    }

    void setTile(vec2f tile_pos) {
        tilePosition = tile_pos;
        shape.setPosition(
            vec2f{tilePosition.x * GRID_SIZEF, tilePosition.y * GRID_SIZEF});
    }

    auto hasItem(const Item &want) const {
        return std::find_if(
            contents.begin(), contents.end(),
            [&want](const Desire &d) { return d.item.name == want.name; });
    }

    void update(sf::Time elapsed) {
        // Desire is empty
        auto is_empty =
            std::remove_if(contents.begin(), contents.end(),
                           [](const Desire &d) { return d.amount == 0; });
        contents.erase(is_empty, contents.end());
        //

        for (Desire d : contents) {
            d.item.update(elapsed);
        }
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(shape);
        for (Desire d : contents) {
            d.item.tilePosition = tilePosition;
            target.draw(d.item);
        }
    }
};
std::vector<std::unique_ptr<Shelf>> shelves;

bool inBounds(vec2i pos) {
    return (pos.x <= WORLD_GRID_SIZE) && (pos.x >= 0) &&
           (pos.y < WORLD_GRID_SIZE) && (pos.y >= 0);
}

bool isSame(vec2i s, vec2i g) { return s.x == g.x && s.y == g.y; }
bool isSame(vec2f s, vec2f g) {
    return (abs(g.x - s.x) < 0.05 && abs(g.y - s.y) < 0.05);
}

bool isCloseEnough(vec2i s, vec2i g, int d) {
    return (abs(g.x - s.x) <= d && abs(g.y - s.y) <= d);
}

bool isCloseEnough(vec2f s, vec2f g, float d) {
    return (abs(g.x - s.x) <= d && abs(g.y - s.y) <= d);
}

bool isValidLocation(vec2i pos) {
    if (!inBounds(pos)) {
        return false;
    }
    for (const std::unique_ptr<Shelf> &s : shelves) {
        if (s->tilePosition.x == pos.x && s->tilePosition.y == pos.y) {
            return false;
        }
    }
    return true;
}

struct VectorHash {
    std::size_t operator()(sf::Vector2<int> a) const {
        return std::hash<int>()(a.x) ^ ((std::hash<int>()(a.y) << 1) >> 1);
    }
};

struct QueueItem {
    vec2i location;
    double score;
    QueueItem(vec2i l, double s) : location(l), score(s) {}
    bool operator<(const QueueItem &s) const { return this->score > s.score; }
};

struct Score {
    double value;
    Score() : value(WORLD_GRID_SIZE * WORLD_GRID_SIZE) {}
    Score(double s) : value(s) {}
    bool operator<(const Score &s) const { return this->value < s.value; }
};

std::vector<vec2i> reconstruct_path(
    const std::unordered_map<vec2i, vec2i, VectorHash> &cameFrom, vec2i cur) {
    std::vector<vec2i> path;
    path.push_back(cur);
    while (cameFrom.find(cur) != cameFrom.end()) {
        cur = cameFrom.at(cur);
        path.push_back(cur);
    }
    return path;
}

std::vector<vec2i> astar(const vec2i &start, const vec2i &goal) {
    auto heuristic = [](const vec2i &s, const vec2i &g) {
        return abs(s.x - g.x) + abs(s.y - g.y);
    };

    std::priority_queue<QueueItem> openSet;
    openSet.push(QueueItem{start, 0});

    std::unordered_map<vec2i, vec2i, VectorHash> cameFrom;
    std::unordered_map<vec2i, double, VectorHash> distTraveled;

    distTraveled[start] = 0;

    while (!openSet.empty()) {
        QueueItem qi = openSet.top();
        openSet.pop();
        auto cur = qi.location;
        if (isSame(cur, goal)) break;

        const int x[] = {0, 0, 1, -1, -1, 1, -1, 1};
        const int y[] = {1, -1, 0, 0, -1, -1, 1, 1};

        for (int i = 0; i < 8; i++) {
            vec2i neighbor = vec2i{cur.x + x[i], cur.y + y[i]};
            double newCost = distTraveled[cur] + dist_i(cur, neighbor);
            if (distTraveled.find(neighbor) == distTraveled.end() ||
                newCost < distTraveled[neighbor]) {
                distTraveled[neighbor] = newCost;
                double prio = newCost + heuristic(neighbor, goal);
                openSet.push(QueueItem{neighbor, prio});
                cameFrom[neighbor] = cur;
            }
        }
    }
    return reconstruct_path(cameFrom, goal);
}

std::vector<std::unique_ptr<Shelf>>::iterator shelf_with_item(
    const Item &want) {
    std::vector<std::unique_ptr<Shelf>>::iterator it = std::find_if(
        shelves.begin(), shelves.end(),
        [&want](const std::unique_ptr<Shelf> &s) {
            auto itt = std::find_if(
                s->contents.begin(), s->contents.end(),
                [&want](const Desire &d) { return d.item.name == want.name; });
            return itt != s->contents.end();
        });
    return it;
}

std::vector<std::unique_ptr<Shelf>>::iterator closest_shelf_to_current_pos(
    vec2f pos) {
    return std::find_if(shelves.begin(), shelves.end(),
                        [&pos](const std::unique_ptr<Shelf> &s) {
                            return isCloseEnough(vtoi(pos),
                                                 vtoi(s->tilePosition), 1);
                        });
}

struct Person : public sf::Drawable {
    sf::RectangleShape shape;
    vec2f tilePosition;

    Person() {
        shape = sf::RectangleShape(vec2f{GRID_SIZEF / 2, GRID_SIZEF / 2});
        shape.setFillColor(sf::Color::Red);
    }

    void setFillColor(sf::Color c) { shape.setFillColor(c); }

    vec2f move_toward_target(vec2f pos, vec2f t, float px) {
        double xdiff = t.x - pos.x;
        double ydiff = t.y - pos.y;
        if (xdiff < -px)
            pos.x -= px;
        else if (xdiff > px)
            pos.x += px;
        if (ydiff < -px)
            pos.y -= px;
        else if (ydiff > px)
            pos.y += px;
        return pos;
    }

    void setTile(vec2f tile_pos) {
        tilePosition = tile_pos;
        shape.setPosition(
            vec2f{tilePosition.x * GRID_SIZEF, tilePosition.y * GRID_SIZEF});
    }

    void update(sf::Time dt) {}

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(shape);
    }
};

struct PersonWithDesire : public Person {
    Desires desires;

    int desireIndex = -1;
    vec2f desireLocation;
    std::vector<vec2i> followPath;

    void add_desire(const Desire &d) { desires.push_back(d); }

    void clean_up_desires() {
        desires.erase(
            std::remove_if(desires.begin(), desires.end(),
                           [](const Desire &d) { return d.amount == 0; }),
            desires.end());
    }

    virtual vec2f location_of_desire() = 0;

    virtual void run_shelf_action(vec2f pos) = 0;

    void move_toward_desire_location() {
        if (desires.size() == 0) {
            tilePosition =
                move_toward_target(tilePosition, vec2f{0.f, 0.f}, 0.05);
            setTile(tilePosition);
            // TODO go to checkout when done
            return;
        }

        // we have no selected desire
        if (desireIndex == -1) {
            desireLocation = location_of_desire();
        }

        // are we at the shelf yet?
        if (isSame(tilePosition, desireLocation)) {
            run_shelf_action(tilePosition);
        }

        if (followPath.size() == 0) {
            followPath = astar(vtoi(tilePosition), vtoi(desireLocation));
            return;
        }

        auto local_target = vtof(followPath.back());
        tilePosition = move_toward_target(tilePosition, local_target, 0.05);
        setTile(tilePosition);
        if (isSame(tilePosition, local_target)) followPath.pop_back();
    }

    void update(sf::Time dt) {
        Person::update(dt);
        clean_up_desires();
        move_toward_desire_location();
    }
};

struct StockClerk : public PersonWithDesire {
    StockClerk() {
        setTile(randVecf(0, WORLD_GRID_SIZE));

        Desire d(all_items[0], 10);
        add_desire(d);
        setFillColor(sf::Color::Blue);
    }

    void run_shelf_action(vec2f pos) {
        if (shelves.empty()) return;

        auto shelf_it = closest_shelf_to_current_pos(pos);
        // Does this shelf already have this item?
        Desire des = desires[desireIndex];
        const std::unique_ptr<Shelf> &shelf = *shelf_it;

        bool found = false;
        for (int i = 0; i < shelf->contents.size(); i++) {
            if (shelf->contents[i].item.name == des.item.name) {
                shelf->contents[i].amount += des.amount;
                found = true;
            }
        }

        if (!found) {
            shelf->contents.push_back(Desire(des));
        }

        // maybe we want to have a max stack size or something
        // and not add everything
        des.amount = 0;
        // write back so it can be cleaned up
        desires[desireIndex] = des;
        desireIndex = -1;
    }

    vec2f location_of_desire() {
        // find any empty shelves first
        std::vector<std::unique_ptr<Shelf>>::iterator it =
            std::find_if(shelves.begin(), shelves.end(),
                         [](const std::unique_ptr<Shelf> &s) {
                             return s->contents.empty();
                         });
        if (it != shelves.end()) {
            desireIndex = 0;
            return (*it)->tilePosition;
        }

        // TODO - wander?
        // just send them to the top left and wait
        return vec2f{0, 0};
    }
};

struct Customer : public PersonWithDesire {
    Customer() {
        setTile(randVecf(0, WORLD_GRID_SIZE));

        Desire d(all_items[0], 1);
        add_desire(d);
    }

    vec2f location_of_desire() {
        for (int i = 0; i < desires.size(); i++) {
            Item item = desires[i].item;
            auto shelf_it = shelf_with_item(item);
            if (shelf_it != shelves.end()) {
                desireIndex = i;
                return (*shelf_it)->tilePosition;
            } else {
                // std::cout << " no shelf with what i want " << item.name
                // << std::endl;
            }
        }
        // TODO replace with pain or wander
        // didnt find any shelves, just go somewhere for now
        return randVecf(0, WORLD_GRID_SIZE);
    }

    void run_shelf_action(vec2f pos) {
        Desire des = desires[desireIndex];

        // find the closest shelf
        auto shelf_it = closest_shelf_to_current_pos(pos);
        if (shelf_it != shelves.end()) {
            // found a matching shelf
            std::vector<Desire>::iterator itt = std::find_if(
                (*shelf_it)->contents.begin(), (*shelf_it)->contents.end(),
                [&](const Desire &d) { return d.item.name == des.item.name; });
            if (itt != (*shelf_it)->contents.end()) {
                // Shelf still has the item we want
                // do they have enough?
                if (itt->amount >= des.amount) {
                    itt->amount -= des.amount;
                    des.amount = 0;
                } else {
                    // they dont have enough....
                    des.amount -= itt->amount;
                    // take everything
                    itt->amount = 0;
                }
                desires[desireIndex] = des;
            } else {
                // shelf didnt have our item at all
            }
        }
        desireIndex = -1;
    }
};

struct World : public sf::Drawable {
    vec2i mousePosWindow;
    vec2f mousePosView;
    vec2i mousePosGrid;
    sf::RectangleShape gridOutline;

    std::vector<Customer *> customers;
    std::vector<PersonWithDesire *> employees;
    Desires globalStock;
    Desires globalForPurchase;
    Desires globalWants;
    int numIdle;

    World() {
        tilemap.resize(WORLD_GRID_SIZE, std::vector<Tile>());
        for (int i = 0; i < WORLD_GRID_SIZE; i++) {
            tilemap[i].resize(WORLD_GRID_SIZE, Tile());
            for (int j = 0; j < WORLD_GRID_SIZE; j++) {
                tilemap[i][j].setPosition(
                    vec2f{i * GRID_SIZEF, j * GRID_SIZEF});
            }
        }

        gridOutline = sf::RectangleShape(vec2f{GRID_SIZE, GRID_SIZE});
        gridOutline.setFillColor(sf::Color::Transparent);
        gridOutline.setOutlineThickness(1.0f);
        gridOutline.setOutlineColor(sf::Color::Green);

        for (int i = 0; i < 10; i++) {
            customers.push_back(new Customer());
        }

        for (int i = 0; i < 1; i++) {
            employees.push_back(new StockClerk());
        }

        for (int i = 0; i < 5; i++) {
            std::unique_ptr<Shelf> s(new Shelf());
            s->setTile(randVecf(0, WORLD_GRID_SIZE - 1));
            shelves.push_back(std::move(s));
        }
        numIdle = 0;
    }

    ~World() {
        for (int i = customers.size() - 1; i >= 0; i--) {
            delete customers[i];
        }

        for (int i = employees.size() - 1; i >= 0; i--) {
            delete employees[i];
        }
    }

    void update(sf::Time dt) {
        gridOutline.setPosition(mousePosGrid.x * GRID_SIZE,
                                mousePosGrid.y * GRID_SIZE);
        globalStock.clear();
        globalForPurchase.clear();
        globalWants.clear();
        numIdle = 0;

        // What supplies do we have (including in the back)
        for (int i = 0; i < employees.size(); i++) {
            employees[i]->update(dt);

            for (const Desire &d : employees[i]->desires) {
                auto it = std::find_if(globalStock.begin(), globalStock.end(),
                                       [&](const Desire &des) {
                                           return d.item.name == des.item.name;
                                       });
                if (it == globalStock.end()) {
                    globalStock.push_back(d);
                } else {
                    it->amount += d.amount;
                }
            }
        }

        // Whats out on shelves
        for (int i = 0; i < shelves.size(); i++) {
            shelves[i]->update(dt);

            for (const Desire &d : shelves[i]->contents) {
                auto it = std::find_if(globalForPurchase.begin(),
                                       globalForPurchase.end(),
                                       [&](const Desire &des) {
                                           return d.item.name == des.item.name;
                                       });
                if (it == globalForPurchase.end()) {
                    globalForPurchase.push_back(d);
                } else {
                    it->amount = it->amount + d.amount;
                }
            }
        }

        for (int i = 0; i < customers.size(); i++) {
            customers[i]->update(dt);

            if (customers[i]->desires.size() == 0) {
                numIdle++;
            }

            for (const Desire &d : customers[i]->desires) {
                auto it = std::find_if(globalWants.begin(), globalWants.end(),
                                       [&](const Desire &des) {
                                           return d.item.name == des.item.name;
                                       });
                if (it == globalWants.end()) {
                    globalWants.push_back(d);
                } else {
                    it->amount += d.amount;
                }
            }
        }
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        int sx = 14;
        int sy = 12;

        int colS = (game_view->getCenter().x / GRID_SIZEF) - sx;
        int colE = (game_view->getCenter().x / GRID_SIZEF) + sx;
        int rowS = (game_view->getCenter().y / GRID_SIZEF) - sy;
        int rowE = (game_view->getCenter().y / GRID_SIZEF) + sy;

        if (colS < 0) colS = 0;
        if (colS > WORLD_GRID_SIZE) colS = WORLD_GRID_SIZE - 1;
        if (rowS < 0) rowS = 0;
        if (rowS > WORLD_GRID_SIZE) rowS = WORLD_GRID_SIZE - 1;

        if (colE < 0) colE = 0;
        if (colE > WORLD_GRID_SIZE) colE = WORLD_GRID_SIZE - 1;
        if (rowE < 0) rowE = 0;
        if (rowE > WORLD_GRID_SIZE) rowE = WORLD_GRID_SIZE - 1;

        for (int i = colS; i < colE; i++) {
            for (int j = rowS; j < rowE; j++) {
                target.draw(tilemap[i][j]);
            }
        }

        // TODO this doesnt work right now :(
        target.draw(gridOutline);

        for (int i = 0; i < shelves.size(); i++) {
            target.draw(*shelves[i]);
        }

        for (int i = 0; i < customers.size(); i++) {
            target.draw(*customers[i]);
        }

        for (int i = 0; i < employees.size(); i++) {
            target.draw(*employees[i]);
        }
    }

    void processEvent(sf::RenderWindow &app, sf::Event event) {
        mousePosWindow = sf::Mouse::getPosition(app);
        mousePosView = app.mapPixelToCoords(mousePosWindow);
        mousePosGrid = vec2i{
            (int)mousePosView.x / GRID_SIZE,
            (int)mousePosView.y / GRID_SIZE,
        };

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == KEYS["1"])
                customers.push_back(new Customer());
            if (event.key.code == KEYS["2"])
                employees.push_back(new StockClerk());
        }
    }
};

struct UIText : public sf::Drawable {
    sf::Text content;

    UIText(float x, float y, std::string t = "", int size = 24,
           sf::Color color = sf::Color::Red,
           sf::Text::Style style = sf::Text::Regular) {
        content.setFont(*getGlobalFont());
        content.setString(t);
        content.setCharacterSize(size);
        content.setFillColor(color);
        content.setStyle(style);
        content.setPosition(vec2f{x, y});
    }

    void update(sf::Time dt) {}

    void setContentString(std::string update) { content.setString(update); }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(content);
    }
};

constexpr int H_PADDING = (WIDTH / 50);
constexpr int V_PADDING = (HEIGHT / 50);
constexpr int V_SPACING = (HEIGHT / 45);

struct HUD : public sf::Drawable {
    World *world;

    typedef std::pair<std::string, UIText *> strUI;
    std::map<std::string, UIText *> uitext;

    HUD() {}

    void setWorld(World *w) { world = w; }

    void update(sf::Time dt) {
        if (world == nullptr) {
            return;
        }
        uitext.clear();

        uitext.insert(strUI("idle_shoppers",
                            new UIText(H_PADDING, V_PADDING + V_SPACING)));
        uitext["idle_shoppers"]->setContentString(
            fmt::format("Idle Shoppers: {}", world->numIdle));

        if (!world->globalStock.empty()) {
            uitext.insert(strUI(
                "stock",
                new UIText(H_PADDING,
                           V_PADDING + (V_SPACING * (uitext.size() + 1)))));
            uitext["stock"]->setContentString("Available Stock: ");
            for (const Desire &d : world->globalStock) {
                uitext.insert(strUI(
                    "supply" + d.item.name,
                    new UIText(H_PADDING,
                               V_PADDING + (V_SPACING * (uitext.size() + 1)))));
                uitext["supply" + d.item.name]->setContentString(
                    fmt::format("{} ({})", d.item.name, d.amount));
            }
        }

        if (!world->globalForPurchase.empty()) {
            uitext.insert(strUI(
                "shelved",
                new UIText(H_PADDING,
                           V_PADDING + (V_SPACING * (uitext.size() + 1)))));
            uitext["shelved"]->setContentString("Out on Shelves: ");

            for (const Desire &d : world->globalForPurchase) {
                uitext.insert(strUI(
                    "shelved" + d.item.name,
                    new UIText(H_PADDING,
                               V_PADDING + (V_SPACING * (uitext.size() + 1)))));
                uitext["shelved" + d.item.name]->setContentString(
                    fmt::format("{} ({})", d.item.name, d.amount));
            }
        }

        if (!world->globalWants.empty()) {
            uitext.insert(strUI(
                "wanted_header",
                new UIText(H_PADDING,
                           V_PADDING + (V_SPACING * (uitext.size() + 1)))));
            uitext["wanted_header"]->setContentString("Wants:");

            for (const Desire &d : world->globalWants) {
                uitext.insert(strUI(
                    d.item.name,
                    new UIText(H_PADDING,
                               V_PADDING + (V_SPACING * (uitext.size() + 1)))));
                uitext[d.item.name]->setContentString(
                    fmt::format("{} ({})", d.item.name, d.amount));
            }
        }
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        for (strUI text : uitext) {
            target.draw(*text.second);
        }
    }

    void processEvent(sf::RenderWindow &app, sf::Event event) {
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Space:
                    break;
                default:
                    break;
            }
        }
    }
};

struct GameScreen {
    World *world;
    sf::View view;
    HUD hud;
    int zoom_level;
    float zoom_speed = 0.01;
    int min_zoom = -100;
    int max_zoom;

    GameScreen() {
        max_zoom = DEBUG ? 1000 : 50;
        world = new World();
        setGlobalWorld(world);
        hud.setWorld(world);

        view.reset(sf::FloatRect(0, 0, WIDTH, HEIGHT));
        zoom_level = 1.0f;
        game_view = &view;
    }

    ~GameScreen() { delete world; }

    void process_subscreen_events(sf::RenderWindow &app, sf::Event event) {
        world->processEvent(app, event);
        hud.processEvent(app, event);
    }

    void update_subscreens(sf::Time elapsed) {
        world->update(elapsed);
        hud.update(elapsed);
    }

    void handle_input(const double &dt) {
        double viewSpeed = zoom_level > 0 ? 1500.f : 500.f;
        double xd = 0.f, yd = 0.f;

        if (sf::Keyboard::isKeyPressed(KEYS["Left"])) xd = -viewSpeed * dt;
        if (sf::Keyboard::isKeyPressed(KEYS["Right"])) xd = viewSpeed * dt;
        if (sf::Keyboard::isKeyPressed(KEYS["Up"])) yd = -viewSpeed * dt;
        if (sf::Keyboard::isKeyPressed(KEYS["Down"])) yd = viewSpeed * dt;

        if (sf::Keyboard::isKeyPressed(KEYS["Scroll-In"]) &&
            zoom_level < max_zoom) {
            view.zoom(1 + zoom_speed);
            zoom_level += 1;
        }
        if (sf::Keyboard::isKeyPressed(KEYS["Scroll-Out"]) &&
            zoom_level > min_zoom) {
            view.zoom(1 - zoom_speed);
            zoom_level -= 1;
        }

        view.move(xd, yd);
    }

    virtual int run(sf::RenderWindow &app) {
        bool running = true;
        sf::Clock tickTimer;
        sf::Time elapsed;
        sf::Event event;

        view.setSize(app.getSize().x, app.getSize().y);
        view.setCenter(app.getSize().x / 2.0f, app.getSize().y / 2.0f);

        while (running) {
            elapsed = tickTimer.restart();
            const double dt = elapsed.asSeconds();

            while (app.pollEvent(event)) {
                process_subscreen_events(app, event);
                // Window closed
                if (event.type == sf::Event::Closed) return (-1);
                if (event.type == sf::Event::MouseWheelMoved) {
                    const int delta = event.mouseWheel.delta;
                    if (delta < 0 && zoom_level < max_zoom) {
                        view.zoom(1 + zoom_speed);
                        zoom_level += 2;
                    } else if (delta > 0 && zoom_level > min_zoom) {
                        view.zoom(1 - zoom_speed);
                        zoom_level -= 2;
                    }
                }
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                        case sf::Keyboard::Escape:
                            return MenuState::PAUSED;
                        default:
                            break;
                    }
                }
            }

            // this is the non event based input
            handle_input(dt);
            // update all subscreens now that events are done
            update_subscreens(elapsed);
            app.clear(sf::Color::Black);
            app.setView(view);
            app.draw(*world);
            // set back to normal 1080p (util.h) size
            // before drawing the HUD so it is scaled correctly
            app.setView(app.getDefaultView());
            app.draw(hud);
            // output to screen
            app.display();
        }
        return -1;
    }
};

int main() {
    sf::Font font;
    font.loadFromFile("./resources/Roboto-Regular.ttf");
    setGlobalFont(&font);

    load_keys();
    // TODO move export to save options
    export_keys();

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Ahoy!");
    window.setFramerateLimit(60);

    GameScreen gs;
    gs.run(window);
    return 0;
}

