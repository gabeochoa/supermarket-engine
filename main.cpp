
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
constexpr int WORLD_GRID_SIZE = 20;

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

    Item(std::string n, double p) : name(n), price(p) {}
    bool operator<(const Item &i) const { return this->name < i.name; }

    void update(sf::Time dt) {}

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        sf::RectangleShape s =
            sf::RectangleShape(vec2f{GRID_SIZEF / 4, GRID_SIZEF / 4});
        s.setPosition(
            vec2f{tilePosition.x * GRID_SIZEF, tilePosition.y * GRID_SIZEF});
        s.setFillColor(sf::Color::Green);
        target.draw(s);
    }
};

const std::vector<Item> all_items = std::vector<Item>{
    Item("apple", 1.0),
};

struct Desire {
    Item item;
    int amount;
    Desire(Item i, int a) : item(i), amount(a) {}
};
typedef std::vector<Desire> Desires;

struct Shelf : public sf::Drawable {
    Desires contents;
    vec2f tilePosition;

    sf::RectangleShape shape;

    Shelf() {
        shape = sf::RectangleShape(vec2f{GRID_SIZEF, GRID_SIZEF});
        shape.setFillColor(sf::Color::Black);

        contents.push_back(Desire(all_items[0], 1));
    }

    void setTile(vec2f tile_pos) {
        tilePosition = tile_pos;
        shape.setPosition(
            vec2f{tilePosition.x * GRID_SIZEF, tilePosition.y * GRID_SIZEF});
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
std::vector<Shelf> shelves;

bool inBounds(vec2i pos) {
    return (pos.x <= WORLD_GRID_SIZE) && (pos.x >= 0) &&
           (pos.y < WORLD_GRID_SIZE) && (pos.y >= 0);
}

bool isSame(vec2i s, vec2i g) { return s.x == g.x && s.y == g.y; }
bool isSame(vec2f s, vec2f g) {
    return (abs(g.x - s.x) < 0.05 && abs(g.y - s.y) < 0.05);
}

bool isValidLocation(vec2i pos) {
    if (!inBounds(pos)) {
        return false;
    }
    for (const Shelf &s : shelves) {
        if (s.tilePosition.x == pos.x && s.tilePosition.y == pos.y) {
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

std::vector<Shelf>::iterator shelf_with_item(const Item &want) {
    std::vector<Shelf>::iterator it =
        std::find_if(shelves.begin(), shelves.end(), [&want](const Shelf &s) {
            auto itt = std::find_if(
                s.contents.begin(), s.contents.end(),
                [&want](const Desire &d) { return d.item.name == want.name; });
            return itt != s.contents.end();
        });
    return it;
}

struct Person : public sf::Drawable {
    sf::RectangleShape shape;
    vec2f tilePosition;

    Person() {
        shape = sf::RectangleShape(vec2f{GRID_SIZEF / 2, GRID_SIZEF / 2});
        shape.setFillColor(sf::Color::Red);
    }

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

    vec2f location_of_desire() {
        for (int i = 0; i < desires.size(); i++) {
            Item item = desires[i].item;
            auto shelf_it = shelf_with_item(item);
            if (shelf_it != shelves.end()) {
                desireIndex = i;
                return shelf_it->tilePosition;
            } else {
                std::cout << " no shelf with what i want " << item.name
                          << std::endl;
            }
        }
        // TODO replace with pain or wander
        // didnt find any shelves, just go somewhere for now
        return randVecf(0, WORLD_GRID_SIZE);
    }
    void move_toward_desire_location() {
        // no desires TODO - handle this
        if (desires.size() == 0) return;

        // we have no selected desire
        if (desireIndex == -1) {
            desireLocation = location_of_desire();
            std::cout << fmt::format("my location {} {} des location {} {}",
                                     tilePosition.x, tilePosition.y,
                                     desireLocation.x, desireLocation.y)
                      << std::endl;
        }

        // are we at the shelf yet?
        if (isSame(tilePosition, desireLocation)) {
            // amazing gotta do something, but instead imma just rando the
            // location
            desireLocation = randVecf(0, WORLD_GRID_SIZE);
            std::cout << fmt::format("my location {} {} des location {} {}",
                                     tilePosition.x, tilePosition.y,
                                     desireLocation.x, desireLocation.y)
                      << std::endl;
        }

        if (followPath.size() == 0) {
            followPath = astar(vtoi(tilePosition), vtoi(desireLocation));
            if (followPath.size() != 0) {
                // std::cout << "got an actual path to follow " << std::endl;
                // for (vec2i f : followPath) {
                // std::cout << f.x << ", " << f.y << std::endl;
                // }
            }
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

struct Customer : public PersonWithDesire {
    Customer() {
        Desire d(all_items[0], 1);
        add_desire(d);
    }
};

struct World : public sf::Drawable {
    vec2i mousePosWindow;
    vec2f mousePosView;
    vec2i mousePosGrid;

    sf::RectangleShape gridOutline;

    std::vector<Customer *> customers;

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

        for (int i = 0; i < 10; i++) {
            Shelf s = Shelf();
            s.setTile(randVecf(0, WORLD_GRID_SIZE - 1));
            shelves.push_back(s);
        }
    }

    ~World() {
        for (int i = customers.size() - 1; i >= 0; i--) {
            delete customers[i];
        }
    }

    void update(sf::Time dt) {
        gridOutline.setPosition(mousePosGrid.x * GRID_SIZE,
                                mousePosGrid.y * GRID_SIZE);

        for (int i = 0; i < customers.size(); i++) {
            customers[i]->update(dt);
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
            target.draw(shelves[i]);
        }

        for (int i = 0; i < customers.size(); i++) {
            target.draw(*customers[i]);
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
            switch (event.key.code) {
                case sf::Keyboard::Space:
                    break;
                default:
                    break;
            }
        }
    }
};

struct HUD : public sf::Drawable {
    World *world;

    HUD() {}

    void setWorld(World *w) { world = w; }

    void update(sf::Time dt) {}

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
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

