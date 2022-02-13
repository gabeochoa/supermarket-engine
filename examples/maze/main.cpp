
#include <optional>

#include "../../engine/app.h"
#include "../../engine/camera.h"
#include "../../engine/layer.h"
#include "../../engine/pch.hpp"
#include "../../engine/thetastar.h"
#include "../../engine/vecutil.h"
#include "colors.h"

////////////////////////////////////////
//
// example maze to show pathfinding
//
//
// Left as an excercise to the reader ( thats you ! )
// -
//
// Known Bugs
// -
//
////////////////////////////////////////

constexpr int WIN_H = 1080;
constexpr int WIN_W = 1920;

constexpr int TILESIZE = 30;

constexpr int MAP_H = WIN_H / TILESIZE;
constexpr int MAP_W = WIN_W / TILESIZE;
constexpr glm::vec2 tilesize{TILESIZE};

struct Tile {
    glm::vec2 position;
    glm::vec4 color;

    void render() {
        auto size = tilesize;
        auto pos = (position * size) + (size / 2.f);
        auto darker = color / 2.f;
        pos += (size * 0.1f);
        size *= 0.8;
        Renderer::drawQuad(pos, size, color);
        Renderer::drawQuad(pos, size, darker);
    }
};

std::array<Tile, MAP_H * MAP_W> grid;
bool running = true;

inline bool inBound(const glm::vec2& pos) {
    return (pos.x > 0 && pos.y > 0 && pos.x < MAP_W - 1 && pos.y < MAP_H - 1);
}

inline bool isColor(const glm::vec2& pos, const glm::vec4 color) {
    return grid[pos.y * MAP_W + pos.x].color == color;
}

typedef std::vector<glm::vec2> Path;

Path bfs(const glm::vec2& start, const glm::vec2& end) {
    if (start.x == 0 && start.y == 0) return Path();
    if (end.x == 0 && end.y == 0) return Path();

    std::array<bool, MAP_H * MAP_W> visited;
    for (int i = 0; i < MAP_W; i++) {
        for (int j = 0; j < MAP_H; j++) {
            visited[j * MAP_W + i] = false;
        }
    }

    std::queue<Path> q;
    q.push(Path{start});

    while (!q.empty()) {
        const Path p = q.front();
        q.pop();
        glm::vec2 pos = p.back();
        if (pos == end) return p;
        visited[pos.y * MAP_W + pos.x] = true;
        log_info("visiting: {}", pos);
        //
        const float x[4] = {0, 0, 1, -1};
        const float y[4] = {1, -1, 0, 0};
        for (int i = 0; i < 4; i++) {
            glm::vec2 neighbor = {pos.x + x[i], pos.y + y[i]};
            if (!inBound(neighbor) ||       //
                isColor(neighbor, GRAY) ||  //
                visited[neighbor.y * MAP_W + neighbor.x]) {
                continue;
            }
            Path p2(p);
            p2.push_back(neighbor);
            q.push(p2);
        }
    }
    log_info("queue empty");
    return Path();
}

struct DemoLayer : public Layer {
    std::shared_ptr<OrthoCameraController> cameraController;
    // 0 = wall; 1 = start; 2 = end;
    int selectedTool = 0;
    glm::vec2 start;
    glm::vec2 end;
    bool shouldRecalculate = false;
    std::vector<glm::vec2> path;

    virtual ~DemoLayer() {}

    DemoLayer() : Layer("") {
        init_map();
        init_camera();
    }

    virtual void init_camera() {
        cameraController.reset(
            new OrthoCameraController((float)WIN_W / WIN_H, 0.f, 5.f, 5.f));
        cameraController->camera.setViewport(glm::vec4{0, 0, WIN_W, WIN_H});
        cameraController->camera.setPosition(
            glm::vec3{WIN_W / 2.f, WIN_H / 2.f, 0.f});
        cameraController->setZoomLevel(WIN_W * 0.3f);
    }

    void init_map() {
        for (int i = 0; i < MAP_W; i++) {
            for (int j = 0; j < MAP_H; j++) {
                grid[j * MAP_W + i] = Tile({
                    .position = {i, j},
                    .color =
                        (j == 0 || j == MAP_H - 1 || i == 0 || i == MAP_W - 1)
                            ? GRAY
                            : BLACK,
                });
            }
        }
    }

    void render() {
        Renderer::begin(cameraController->camera);
        {
            for (auto t : grid) t.render();
            for (auto pos : path) {
                if (pos == start || pos == end) continue;
                Tile{.position = pos, .color = PURPLE}.render();
            }
        }
        Renderer::end();
    }

    virtual void onUpdate(Time dt) override {
        cameraController->onUpdate(dt);
        render();

        if (shouldRecalculate) {
            shouldRecalculate = false;
            path = bfs(start, end);
            for (auto pos : path) log_info("path: {}", pos);
        }
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        if (event.keycode == Key::KeyCode::D0) selectedTool = 0;
        if (event.keycode == Key::KeyCode::D1) selectedTool = 1;
        if (event.keycode == Key::KeyCode::D2) selectedTool = 2;

        if (event.keycode == Key::KeyCode::Delete)
            selectedTool = (selectedTool + 1) % 3;

        log_info("selected tool is {}", selectedTool);
        return false;
    }

    glm::vec3 getMouseInWorld() {
        auto mouse = Input::getMousePosition();
        return screenToWorld(glm::vec3{mouse.x, WIN_H - mouse.y, 0.f},
                             cameraController->camera.view,
                             cameraController->camera.projection,
                             cameraController->camera.viewport);
    }

    void setTileColor(glm::vec2 location, glm::vec4 color) {
        Tile& tile = grid[location.y * MAP_W + location.x];
        tile.color = color;
    }

    void place_thing(glm::vec2 location) {
        if (selectedTool == 0) {
            Tile& tile = grid[location.y * MAP_W + location.x];
            if (tile.color == GRAY) tile.color = BLACK;
            if (tile.color == BLACK) tile.color = GRAY;
        }
        if (selectedTool == 1) {
            if (start.x != 0 && start.y != 0) setTileColor(start, BLACK);
            setTileColor(location, BLUE);
            start = location;
        }
        if (selectedTool == 2) {
            if (end.x != 0 && end.y != 0) setTileColor(end, BLACK);
            setTileColor(location, GREEN);
            end = location;
        }

        shouldRecalculate = true;
    }

    bool onMouseButtonPressed(Mouse::MouseButtonPressedEvent& event) {
        if (event.GetMouseButton() == Mouse::MouseCode::ButtonLeft) {
            auto mousePos = getMouseInWorld();
            glm::vec2 tile = glm::vec2{floor(mousePos.x / TILESIZE),
                                       floor(mousePos.y / TILESIZE)};

            int i = tile.x;
            int j = tile.y;
            bool walls = (j == 0 || j == MAP_H - 1 || i == 0 || i == MAP_W - 1);
            if (!walls) {
                place_thing(tile);
            }
        }

        return false;
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&DemoLayer::onKeyPressed, this, std::placeholders::_1));
        dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(std::bind(
            &DemoLayer::onMouseButtonPressed, this, std::placeholders::_1));
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    App::create({
        .width = WIN_W,
        .height = WIN_H,
        .title = " maze 🌽 ",
        .clearEnabled = true,
        .escClosesWindow = true,
        .initResourcesFolder = "../resources",
    });

    Layer* demo = new DemoLayer();
    App::get().pushLayer(demo);

    App::get().run();

    return 0;
}
