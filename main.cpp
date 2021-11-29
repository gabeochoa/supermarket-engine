
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
#include <map>
#include <queue>
#include <sstream>
#include <string>

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
constexpr int WORLD_GRID_SIZE = 50;

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
std::vector<std::vector<Tile> > tilemap;

/// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
struct Item : public sf::Drawable {
    std::string name;
    double price;
    Item(std::string n, double p) : name(n), price(p) {}
    bool operator<(const Item &i) const { return this->name < i.name; }

    void update(sf::Time dt) {}

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        sf::RectangleShape s =
            sf::RectangleShape(vec2f{GRID_SIZEF, GRID_SIZEF});
        s.setFillColor(sf::Color::Green);
        target.draw(s);
    }
};

const std::vector<Item> all_items = std::vector<Item>{
    Item("apple", 1.0),
};

struct Desire {
    Item *item;
    int amount;
    Desire(Item *i, int a) : item(i), amount(a) {}
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

    void update(sf::Time elapsed) {
        for (Desire d : contents) {
            d.item->update(elapsed);
        }
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(shape);
        for (Desire d : contents) {
            target.draw(*(d.item));
        }
    }
};
std::vector<Shelf> shelves;

struct Person : public sf::Drawable {
    sf::RectangleShape shape;
    vec2f tilePosition;

    Person() {
        shape = sf::RectangleShape(vec2f{GRID_SIZEF, GRID_SIZEF});
        shape.setFillColor(sf::Color::Red);
    }

    vec2f move_toward_target(vec2f pos, vec2f t, float px) {
        if (pos.x <= t.x) pos.x += px;
        if (pos.x >= t.x) pos.x -= px;
        if (pos.y <= t.y) pos.y += px;
        if (pos.y >= t.y) pos.y -= px;
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

    void add_desire(const Desire &d) { desires.push_back(d); }

    void clean_up_desires() {
        auto is_complete =
            std::remove_if(desires.begin(), desires.end(),
                           [](const Desire &d) { return d.amount == 0; });
        desires.erase(is_complete, desires.end());
    }

    vec2f location_of_desire() {
        desireIndex = 0;
        // TODO find the location of the correct shelf
        return vec2f{
            1.0f * randIn(0, WORLD_GRID_SIZE),
            1.0f * randIn(0, WORLD_GRID_SIZE),
        };
    }
    void move_toward_desire_location() {
        // no desires TODO - handle this
        if (desires.size() == 0) return;
        if (desireIndex == -1) {
            desireLocation = location_of_desire();
            std::cout << fmt::format("my location {} {} des location {} {}",
                                     tilePosition.x, tilePosition.y,
                                     desireLocation.x, desireLocation.y)
                      << std::endl;
            return;
        }

        tilePosition = move_toward_target(tilePosition, desireLocation, 0.05);
        setTile(tilePosition);
    }

    void update(sf::Time dt) {
        Person::update(dt);
        clean_up_desires();
        move_toward_desire_location();
    }
};

struct Customer : public PersonWithDesire {
    Customer() {
        auto apple = all_items[0];
        Desire d(&apple, 1);
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

        Customer *cust = new Customer();
        cust->setTile(vec2f{5, 5});
        customers.push_back(cust);

        for (int i = 0; i < 1; i++) {
            Shelf s = Shelf();
            s.setTile(vec2f{
                (float)1.0f * randIn(10, WORLD_GRID_SIZE),
                (float)1.0f * randIn(10, WORLD_GRID_SIZE),
            });
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

        for (int i = 0; i < customers.size(); i++) {
            target.draw(*customers[i]);
        }

        for (int i = 0; i < shelves.size(); i++) {
            target.draw(shelves[i]);
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
    int max_zoom = 50;

    GameScreen() {
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

