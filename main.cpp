
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
constexpr int GRID_SIZE = 100;
constexpr float GRID_SIZEF = 100.f;
// number of grid items in the world
constexpr int WORLD_GRID_SIZE = 1000;

struct World : public sf::Drawable {
    std::vector<std::vector<sf::RectangleShape> > tilemap;

    World() {
        tilemap.resize(WORLD_GRID_SIZE, std::vector<sf::RectangleShape>());
        for (int i = 0; i < WORLD_GRID_SIZE; i++) {
            tilemap[i].resize(WORLD_GRID_SIZE, sf::RectangleShape());
            for (int j = 0; j < WORLD_GRID_SIZE; j++) {
                tilemap[i][j].setSize(vec2f{GRID_SIZEF, GRID_SIZEF});
                tilemap[i][j].setFillColor(sf::Color::White);
                tilemap[i][j].setOutlineThickness(1.0f);
                tilemap[i][j].setOutlineColor(sf::Color::Black);
                tilemap[i][j].setPosition(i * GRID_SIZEF, j * GRID_SIZEF);
            }
        }
    }

    void update(sf::Time dt) {}

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        int colS = (game_view->getCenter().x / GRID_SIZEF) - 12;
        int colE = (game_view->getCenter().x / GRID_SIZEF) + 12;
        int rowS = (game_view->getCenter().y / GRID_SIZEF) - 8;
        int rowE = (game_view->getCenter().y / GRID_SIZEF) + 8;

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

struct HUD : public sf::Drawable {
    World *world;
    vec2i mousePosWindow;
    vec2f mousePosView;
    vec2i mousePosGrid;

    sf::RectangleShape gridOutline;

    HUD() {
        gridOutline = sf::RectangleShape(vec2f{GRID_SIZE, GRID_SIZE});
        gridOutline.setFillColor(sf::Color::Transparent);
        gridOutline.setOutlineThickness(1.0f);
        gridOutline.setOutlineColor(sf::Color::Green);
    }

    void setWorld(World *w) { world = w; }

    void update(sf::Time dt) {
        gridOutline.setPosition(mousePosGrid.x * GRID_SIZE,
                                mousePosGrid.y * GRID_SIZE);
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(gridOutline);
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

struct GameScreen {
    World *world;
    sf::View view;
    HUD hud;

    GameScreen() {
        world = new World();
        hud.setWorld(world);
        view.reset(sf::FloatRect(0, 0, WIDTH, HEIGHT));
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
        const double viewSpeed = 100.f;
        double xd = 0.f;
        double yd = 0.f;
        if (sf::Keyboard::isKeyPressed(KEYS["Left"])) xd = -viewSpeed * dt;
        if (sf::Keyboard::isKeyPressed(KEYS["Right"])) xd = viewSpeed * dt;
        if (sf::Keyboard::isKeyPressed(KEYS["Up"])) yd = -viewSpeed * dt;
        if (sf::Keyboard::isKeyPressed(KEYS["Down"])) yd = viewSpeed * dt;

        view.move(xd, yd);
    }

    virtual int run(sf::RenderWindow &app) {
        bool running = true;
        sf::Clock tickTimer;
        sf::Time elapsed;
        sf::Event event;

        setGlobalWorld(world);

        view.setSize(app.getSize().x, app.getSize().y);
        view.setCenter(app.getSize().x / 2.0f, app.getSize().y / 2.0f);

        while (running) {
            elapsed = tickTimer.restart();
            const double dt = elapsed.asSeconds();

            while (app.pollEvent(event)) {
                process_subscreen_events(app, event);
                // Window closed
                if (event.type == sf::Event::Closed) return (-1);
                // Key pressed
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

