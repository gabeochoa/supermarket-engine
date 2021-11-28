
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

class IScreen {
   public:
    virtual int run(sf::RenderWindow &App) = 0;
};

#include "entity.hpp"

struct Item {
    std::string name;
    double price;
    Item(std::string n, double p) : name(n), price(p) {}
    bool operator<(const Item &i) const { return this->name < i.name; }
};

const std::vector<Item> all_items = std::vector<Item>{
    Item("apple", 1.0), Item("potato", 1.0), Item("cabbage", 1.0),
    Item("milk", 1.0),  Item("eggs", 1.0),   Item("bread", 1.0),
};

typedef std::pair<Item, int> desire;
typedef std::vector<desire> Desires;

std::vector<sf::FloatRect> nonWalkables;

//////////// //////////// //////////// ////////////
//////////// //////////// //////////// ////////////
//////////// //////////// //////////// ////////////

constexpr static int PWIDTH = 60;
constexpr static int PHEIGHT = 32;
constexpr static double MAX_VALUE = PWIDTH * PHEIGHT;

bool inBounds(vec2f pos) {
    return (pos.x <= PWIDTH) && (pos.x >= 0) && (pos.y < PHEIGHT) &&
           (pos.y >= 0);
}

bool closeEnough(vec2f pos, vec2f t, double e = 2.0) {
    return ((pos.x < t.x + e) && (pos.x > t.x - e) && (pos.y < t.y + e) &&
            (pos.y > t.y - e));
}

bool isValidLocation(vec2f s) {
    // on screen
    if (!inBounds(s)) {
        return false;
    }
    // not inside the shelves
    for (sf::FloatRect fr : nonWalkables) {
        if (fr.contains(s.x, s.y)) {
            return false;
        }
    }
    return true;
}

bool isDest(vec2f s, vec2f g) { return closeEnough(s, g, 2); }

double heuristic(vec2f s, vec2f g) { return dist(s, g); }

struct Score {
    double value;
    Score() : value(MAX_VALUE) {}
    Score(double s) : value(s) {}
    bool operator<(const Score &s) const { return this->value < s.value; }
};

struct VectorComparator {
    bool operator()(sf::Vector2f lhs, sf::Vector2f rhs) const {
        return lhs.x + lhs.y < rhs.x + rhs.y;
    }
};

struct QueueItem {
    double score;
    vec2f location;
    bool operator<(const QueueItem &s) const { return this->score < s.score; }
};

std::vector<vec2f> reconstruct_path(
    std::map<vec2f, vec2f, VectorComparator> cameFrom, vec2f cur) {
    std::vector<vec2f> path;
    path.push_back(cur);
    while (cameFrom.find(cur) != cameFrom.end()) {
        cur = cameFrom[cur];
        path.push_back(cur);
    }
    for (auto s : path) {
        std::cout << fmt::format(" path: ({}, {}) ", s.x, s.y) << std::endl;
    }
    std::cout << "end path" << std::endl;
    return path;
}

std::vector<vec2f> astar(vec2f start, vec2f goal) {
    if (!isValidLocation(goal)) {
        assert("goal not valid");
    }

    // The set of discovered nodes that may need to be (re-)expanded.
    // Initially, only the start node is known.
    // This is usually implemented as a min-heap or priority queue rather than a
    // hash-set.
    std::vector<QueueItem> openSet;
    // For node n, cameFrom[n] is the node immediately preceding it on the
    // cheapest path from start to n currently known.
    std::map<vec2f, vec2f, VectorComparator> cameFrom;
    // For node n, gScore[n] is the cost of the cheapest path from start to n
    // currently known.
    std::map<vec2f, Score, VectorComparator> gScore;
    gScore[start] = 0;

    // For node n, fScore[n] := gScore[n] + h(n). fScore[n] represents our
    // current best guess as to how short a path from start to finish can be if
    // it goes through n.
    std::map<vec2f, Score, VectorComparator> fScore;
    fScore[start] = heuristic(start, goal);

    // add starting node into the openSet
    openSet.push_back(QueueItem{fScore[start].value, start});

    while (!openSet.empty()) {
        // This operation can occur in O(1) time if openSet is a min-heap or a
        // priority queue

        // since we cant find in prio queue in cpp, just use a list and sort to
        // keep closest at the top
        sort(openSet.begin(), openSet.end(),
             [=](QueueItem &a, QueueItem &b) { return a.score < b.score; });

        // grab our best guest for the next spot to visit
        vec2f cur = openSet.front().location;
        openSet.erase(openSet.begin());

        // if its the goal, then we good
        if (isDest(cur, goal)) {
            return reconstruct_path(cameFrom, cur);
        }

        const int x[] = {-1, 0, 1, -1, 0, 1, -1, 1};  // used in functions
        const int y[] = {-1, -1, -1, 1, 1, 1, 0, 0};  // 8 shifts to neighbors

        for (int i = 0; i < 8; i++) {
            vec2f neighbor = vec2f{cur.x + x[i], cur.y + y[i]};
            // is our neighbor a valid location that we can traverse?
            if (!isValidLocation(neighbor)) continue;

            // tentative_gScore is the distance from start to the neighbor
            // through cur
            double tentative_gScore = gScore[cur].value + dist(cur, neighbor);
            if (tentative_gScore < gScore[neighbor].value) {
                // This path to neighbor is better than any previous one. Record
                // it!
                cameFrom[neighbor] = cur;
                gScore[neighbor] = tentative_gScore;
                fScore[neighbor] = tentative_gScore + heuristic(neighbor, goal);
                auto it = std::find_if(openSet.begin(), openSet.end(),
                                       [&neighbor](const QueueItem &des) {
                                           return neighbor == des.location;
                                       });
                if (it == openSet.end()) {
                    openSet.push_back(
                        QueueItem{fScore[neighbor].value, neighbor});
                }
            }
        }
    }
    std::cout << " no solution " << std::endl;
    return std::vector<vec2f>();
}

struct Shelf : public Entity {
    Desires contents;

    Shelf(float x, float y) {
        vec2f rectSize = vec2f{2, 1};
        sf::RectangleShape *collider = new sf::RectangleShape(rectSize);
        collider->setPosition(vec2f{x, y});
        collider->setFillColor(sf::Color(0, 150, 150));
        colliders.push_back(collider);

        // TODO replace with actual inventory
        contents.push_back(desire(all_items[0], randIn(1, 30)));
    }

    float getDrawY() const { return colliders[0]->getPosition().y; }

    std::vector<desire>::iterator hasItem(Item want) {
        return std::find_if(
            contents.begin(), contents.end(),
            [&want](const desire &d) { return d.first.name == want.name; });
    }

    void update(sf::Time elapsed) {
        std::cout << fmt::format("Shelf: ( {}, {} ): {}, {}",
                                 colliders[0]->getPosition().x,
                                 colliders[0]->getPosition().y,
                                 contents[0].first.name, contents[0].second);
        std::cout << std::endl;
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        target.draw(*colliders[0]);
    }
};

std::vector<Shelf *> shelves;

struct Customer : Entity {
    Desires desires;
    vec2f global_target;
    std::vector<vec2f> local_targets;
    bool idle = false;
    int target_desire = -1;

    Customer() {
        colliders.push_back(new sf::RectangleShape(vec2f{1, 1}));
        colliders[0]->setPosition(vec2f{1, 1});

        desires.push_back(desire(all_items[0], 1));
        desires.push_back(desire(all_items[0], 1));
        desires.push_back(desire(all_items[0], 1));
        desires.push_back(desire(all_items[0], 1));
        // int numDesires = randIn(1, 100);
        // random_selector<> selector{};
        // while (desires.size() < numDesires) {
        // desires.push_back(desire(selector(all_items), randIn(1, 30)));
        // }
    }

    float getDrawY() const { return colliders[0]->getPosition().y; }

    vec2f moveToward(vec2f pos, vec2f t, float px) {
        if (pos.x < t.x) pos.x += px;
        if (pos.x > t.x) pos.x -= px;
        if (pos.y < t.y) pos.y += px;
        if (pos.y > t.y) pos.y -= px;
        return pos;
    }

    vec2f keepPosInBounds(vec2f pos) {
        if (pos.x >= WIDTH) pos.x = WIDTH;
        if (pos.x < 0) pos.x = 0;
        if (pos.y >= HEIGHT) pos.y = HEIGHT;
        if (pos.y < 0) pos.y = 0;
        return pos;
    }

    std::vector<Shelf *>::iterator shelf_with_item(Item want) {
        auto it = std::find_if(
            shelves.begin(), shelves.end(), [&want](const Shelf *s) {
                auto itt = std::find_if(s->contents.begin(), s->contents.end(),
                                        [&want](const desire &d) {
                                            return d.first.name == want.name;
                                        });
                return itt != s->contents.end();
            });
        return it;
    }

    std::vector<Shelf *>::iterator closest_shelf_to_current_pos(vec2f pos) {
        return std::find_if(shelves.begin(), shelves.end(),
                            [&pos](const Shelf *s) {
                                auto p = s->colliders[0]->getPosition();
                                return closeEnough(pos, p, 1);
                            });
    }

    void clean_up_desires() {
        auto is_complete =
            std::remove_if(desires.begin(), desires.end(),
                           [](const desire &d) { return d.second == 0; });
        desires.erase(is_complete, desires.end());
    }

    void next_local_target(vec2f pos) {
        pos = moveToward(pos, local_targets.back(), 0.1);
        if (closeEnough(pos, local_targets.back(), 0.1)) {
            // TODO probably just replace with index
            local_targets.pop_back();
        }
        colliders[0]->setPosition(pos.x, pos.y);
    }

    void grab_from_shelf(vec2f pos) {
        // find the closest shelf
        auto shelf_it = closest_shelf_to_current_pos(pos);
        if (shelf_it != shelves.end()) {
            // found a matching shelf
            desire des = desires[target_desire];
            const Item want = des.first;
            auto itt = (*shelf_it)->hasItem(want);
            if (itt != (*shelf_it)->contents.end()) {
                // Shelf still has the item we want
                // do they have enough?
                if (itt->second >= des.second) {
                    itt->second -= des.second;
                    des.second = 0;
                } else {
                    // they dont have enough....
                    des.second -= itt->second;
                    // take everything
                    itt->second = 0;
                }
                target_desire = -1;
            } else {
                std::cout << std::endl;
            }
        }
    }

    void update(sf::Time elapsed) {
        idle = false;
        auto pos = colliders[0]->getPosition();

        // remove any items we no longer need
        clean_up_desires();
        if (desires.empty()) {
            idle = true;
            return;
        }

        if (local_targets.size() != 0) {
            return next_local_target(pos);
        }

        // either just started, or are about to grab something from a shelf.
        if (target_desire != -1) grab_from_shelf(pos);

        // if no target? grab the next thing on your list
        int desireIndex = 0;
        while (desireIndex < desires.size()) {
            const Item want = desires[desireIndex].first;
            // TODO check if store even has the item before looping through
            // shelves
            auto matchingShelf = shelf_with_item(want);
            if (matchingShelf != shelves.end()) {
                auto shelf_pos = (*matchingShelf)->colliders[0]->getPosition();
                // so you cant place a spot on the shelf, so we need to go right
                // on the side of it
                target_desire = desireIndex;
                global_target = vec2f{shelf_pos.x - 1, shelf_pos.y};
                break;
            }
            // no shelf had it,
            // try to find the next item on our list
            desireIndex += 1;
        }

        if (desireIndex >= desires.size()) {
            // TODO - handle the case where customer cant find their remaining
            // items
            idle = true;
            return;
        }

        local_targets = astar(pos, global_target);
    }

    void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        // std::cout << fmt::format("Cust: {} {}", desires[0].first.name,
        // desires[0].second)
        // << std::endl;
        target.draw(*colliders[0]);

        // draw target
        sf::RectangleShape t = sf::RectangleShape(vec2f{1, 1});
        t.setFillColor(sf::Color::Blue);
        t.setPosition(global_target);
        target.draw(t);
    }
};

struct Supermarket {
    std::string name = "MySuperMarket";
    std::vector<Item> inventory;
};

struct World : public sf::Drawable {
    double money;

    Supermarket market;
    int num_idle = 0;

    std::vector<Entity *> entities;
    std::vector<desire> desires;

    World() {
        //  TODO read from savefile
        money = 20.0;

        int xshelf_spacing = 4;
        int yshelf_spacing = 1;
        shelves.push_back(
            new Shelf(3 + (1 * xshelf_spacing), 3 + (3 * yshelf_spacing)));
        // for (int i = 0; i < 10; i++) {
        // for (int j = 0; j < 15; j++) {
        // shelves.push_back(
        // new Shelf(3 + (i * xshelf_spacing), 3 + (j * yshelf_spacing)));
        // }
        // }

        entities.insert(entities.end(), shelves.begin(), shelves.end());
        for (Entity *e : entities) {
            nonWalkables.push_back(e->colliders[0]->getGlobalBounds());
        }
    }

    ~World() {
        for (int i = entities.size() - 1; i >= 0; i--) {
            delete entities[i];
        }
    }

    void update(sf::Time dt) {
        desires.clear();

        num_idle = 0;
        for (int i = 0; i < entities.size(); i++) {
            Entity *entity = entities[i];
            entity->update(dt);

            /// Customers only
            if (typeid(*entity) == typeid(Customer)) {
                Customer *cust = dynamic_cast<Customer *>(entity);
                if (cust->idle) {
                    num_idle += 1;
                }
                for (desire d : cust->desires) {
                    // find the matching desire
                    auto it = std::find_if(
                        desires.begin(), desires.end(), [&](const desire &des) {
                            return d.first.name == des.first.name;
                        });
                    if (it == desires.end()) {
                        desires.push_back(d);
                    } else {
                        it->second += d.second;
                    }
                }
            }
        }

        sort(desires.begin(), desires.end(),
             [=](desire &a, desire &b) { return a.second < b.second; });
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        for (int i = 0; i < entities.size(); i++) {
            target.draw(*entities[i]);
        }
    }

    void processEvent(sf::Event event) {
        // player.processEvent(event);
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
                case sf::Keyboard::Space:
                    entities.push_back(new Customer());
                    break;
                default:
                    break;
            }
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

    HUD() {
        uitext.insert(strUI("name", new UIText(H_PADDING, V_PADDING)));
        uitext.insert(
            strUI("money", new UIText(H_PADDING, V_PADDING + V_SPACING)));
        uitext.insert(
            strUI("idle_shoppers",
                  new UIText(H_PADDING, V_PADDING + (V_SPACING * 2))));
        uitext.insert(strUI(
            "top_desired", new UIText(H_PADDING, V_PADDING + (V_SPACING * 3))));
    }

    void setWorld(World *w) { world = w; }

    void update(sf::Time dt) {
        if (world == nullptr) {
            return;
        }
        uitext["name"]->setContentString(fmt::format("{}", world->market.name));
        uitext["money"]->setContentString(
            fmt::format("Money: {:.2f}", world->money));

        uitext["idle_shoppers"]->setContentString(
            fmt::format("Idle Shoppers: {}", world->num_idle));

        if (world->desires.size() >= 1) {
            auto top_wanted = (world->desires.begin())->first.name;
            uitext["top_desired"]->setContentString(
                fmt::format("Most Wanted: {}", top_wanted));
        }
    }

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const {
        for (strUI text : uitext) {
            target.draw(*text.second);
        }
    }
};

struct GameScreen : public IScreen {
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

    virtual int run(sf::RenderWindow &app) {
        sf::Clock tickTimer;
        sf::Event event;
        bool running = true;
        setGlobalWorld(world);

        while (running) {
            sf::Time elapsed = tickTimer.restart();

            while (app.pollEvent(event)) {
                // Window closed
                if (event.type == sf::Event::Closed) {
                    return (-1);
                }

                world->processEvent(event);
                // Key pressed
                if (event.type == sf::Event::KeyPressed) {
                    switch (event.key.code) {
                        case sf::Keyboard::PageDown:
                            DEBUG_show_colliders = !DEBUG_show_colliders;
                            break;
                        case sf::Keyboard::End:
                            break;
                        case sf::Keyboard::Delete:
                            break;
                        case sf::Keyboard::Escape:
                            if (DEBUG) {
                                return -1;
                            }
                            return MenuState::PAUSED;
                        default:
                            break;
                    }
                }
            }
            float zoom_level = 0.04f;
            view.setCenter(25, 10);
            view.setSize(WIDTH * zoom_level, HEIGHT * zoom_level);
            world->update(elapsed);
            hud.update(elapsed);

            app.clear(sf::Color::Black);

            app.setView(view);
            app.draw(*world);
            app.setView(app.getDefaultView());
            app.draw(hud);

            app.display();
        }
        return -1;
    }
};

using namespace sf;

void debug_info(RenderWindow *window, std::string info) {
    sf::Text text;
    text.setFont(*getGlobalFont());
    text.setString(info);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::Red);
    text.setStyle(sf::Text::Bold);
    text.setPosition(200.f, 200.f);
    window->draw(text);
}

int main() {
    sf::Font font;
    font.loadFromFile("./resources/Roboto-Regular.ttf");
    setGlobalFont(&font);
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "Ahoy!");
    window.setFramerateLimit(60);
    Clock tickTimer;

    std::vector<IScreen *> screens;
    int activeScreen = DEBUG ? 0 : 0;

    GameScreen gs;
    screens.push_back(&gs);

    while (activeScreen >= 0) {
        activeScreen = screens[activeScreen]->run(window);
    }

    return 0;
}
