
#include "../../engine/app.h"
#include "../../engine/camera.h"
#include "../../engine/layer.h"
#include "../../engine/pch.hpp"
#include "colors.h"

////////////////////////////////////////
//
// simple snake
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
constexpr float WIN_RATIO = WIN_W * 1.f / WIN_H;
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

struct DemoLayer : public Layer {
    std::shared_ptr<OrthoCameraController> cameraController;
    float moveTimer;
    float timeBetweenMoves = 0.12f;

    struct Snake {
        glm::vec2 position;
        std::vector<glm::vec2> body;
        int facing;
        size_t maxLength;
        bool movedSinceDirChange = false;

        struct Apple {
            glm::vec2 position;
            void render() {
                auto size = tilesize;
                Renderer::drawQuad((position * size) + (size / 2.f),
                                   size * 0.8f, RED);
            }
        } apple;

        Snake() {
            facing = 1;
            maxLength = 1;
            for (int i = 0; i < 3; i++) eat();
            position = glm::vec2{MAP_W / 2, MAP_H / 2};
            move();
        }

        bool inLocation(glm::vec2 pos, bool skip_head = false) {
            int x = skip_head ? 1 : 0;
            for (auto it = body.begin() + x; it != body.end(); it++)
                if (*it == pos) return true;
            return false;
        }

        void eat() {
            if (maxLength++ > MAP_H * MAP_W) running = false;
            do {
                apple.position = {
                    randIn(1, MAP_W - 1),
                    randIn(1, MAP_H - 1),
                };
            } while (inLocation(apple.position));
        }

        void render() {
            apple.render();
            for (auto it = body.rbegin(); it != body.rend(); it++) {
                Renderer::drawQuad((*it * tilesize) + (tilesize / 2.f),
                                   tilesize * 0.8f, GREEN, "white");
            }
        }

        void setFacing(int f) {
            if (!movedSinceDirChange) return;
            movedSinceDirChange = false;
            if (facing != 2 && f == 0) facing = 0;
            if (facing != 3 && f == 1) facing = 1;
            if (facing != 0 && f == 2) facing = 2;
            if (facing != 1 && f == 3) facing = 3;
        }

        void collisions() {
            if (position.y == 0 || position.y == MAP_H - 1) running = false;
            if (position.x == 0 || position.x == MAP_W - 1) running = false;
            if (inLocation(position)) running = false;

            if (position == apple.position) eat();
        }

        void equilibrium() {
            body.push_back(position);
            while (body.size() > maxLength) body.erase(body.begin());
        }

        void move() {
            if (facing == 0) position.y += 1;
            if (facing == 1) position.x += 1;
            if (facing == 2) position.y -= 1;
            if (facing == 3) position.x -= 1;

            collisions();
            equilibrium();

            movedSinceDirChange = true;
        }
    } snake;

    virtual ~DemoLayer() {}

    DemoLayer() : Layer("") {
        init_camera();
        init_map();
    }

    virtual void init_camera() {
        cameraController.reset(
            new OrthoCameraController(WIN_RATIO, 0.f, 5.f, 5.f));
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

    void go(Time dt) {
        if (!running) return;

        moveTimer += dt.s();
        if (moveTimer < timeBetweenMoves) return;
        moveTimer = 0.f;

        snake.move();
    }

    void render() {
        Renderer::begin(cameraController->camera);
        {
            for (auto t : grid) t.render();
            snake.render();
        }
        Renderer::end();
    }

    virtual void onUpdate(Time dt) override {
        cameraController->onUpdate(dt);
        go(dt);
        render();
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        using namespace Key;
        int keycode = event.keycode;
        if (keycode == KeyCode::Up) snake.setFacing(0);
        if (keycode == KeyCode::Right) snake.setFacing(1);
        if (keycode == KeyCode::Down) snake.setFacing(2);
        if (keycode == KeyCode::Left) snake.setFacing(3);
        if (keycode == KeyCode::R) {
            snake = Snake();
            running = true;
        }
        return false;
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&DemoLayer::onKeyPressed, this, std::placeholders::_1));
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    App::create({
        .width = WIN_W,
        .height = WIN_H,
        .title = " snake 🐍",
        .clearEnabled = true,
        .escClosesWindow = true,
        .initResourcesFolder = "../resources",
    });

    Layer* demo = new DemoLayer();
    App::get().pushLayer(demo);

    App::get().run();

    return 0;
}
