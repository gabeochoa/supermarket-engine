
#include "../../engine/app.h"
#include "../../engine/camera.h"
#include "../../engine/layer.h"
#include "../../engine/pch.hpp"

constexpr int TILESIZE = 80;
constexpr int MAP_H = 31;
constexpr int MAP_W = 12;
constexpr int WIN_H = MAP_H * TILESIZE;
constexpr int WIN_W = (MAP_W - 2) * 2 * TILESIZE;
constexpr float WIN_RATIO = WIN_W * 1.f / WIN_H;

struct Tile {
    glm::vec2 position;
    glm::vec4 color;

    void render() {
        auto size = glm::vec2{TILESIZE};
        auto pos = (position * size) + (size / 2.f);
        auto darker = color / 2.f;
        pos += (size * 0.1f);
        size *= 0.8;
        Renderer::drawQuad(pos, size, color, "white");
        Renderer::drawQuad(pos, size, darker, "white");
    }
};
std::array<Tile, MAP_H * MAP_W> grid;

// colors and a way to convert piece type to correct color
#include "colors.h"
// A bunch of binary data representing the pieces and their rotations
#include "piece_data.h"
// struct for any specific piece
#include "piece.h"

struct DemoLayer : public Layer {
    std::shared_ptr<OrthoCameraController> cameraController;
    Piece currentPiece, previewPiece, ghostPiece;
    float moveTimer;
    float timeBetweenMoves = 0.5f;

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
        cameraController->setZoomLevel(WIN_H);
    }

    void init_map() {
        for (int i = 0; i < MAP_W; i++) {
            for (int j = 0; j < MAP_H; j++) {
                grid[j * MAP_W + i] = Tile({
                    .position = {i, j},
                    .color =
                        (j == 0 || i == 0 || i == MAP_W - 1) ? GRAY : BLACK,
                });
            }
        }
        previewPiece.regen({(MAP_W + 1), (MAP_H + 1)});
        currentPiece.regen({(MAP_W / 2), (MAP_H - 1)});
    }

    void set_grid_color(int x, int y, glm::vec4 color) {
        grid[y * MAP_W + x].color = color;
    }

    void lock_piece() {
        int x = currentPiece.position.x;
        int y = currentPiece.position.y;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentPiece.piece[j * 4 + i] == 0) continue;
                set_grid_color((x + i), (y + j), currentPiece.color);
            }
        }
        currentPiece = previewPiece;
        currentPiece.position = {(MAP_W / 2), (MAP_H)};
        previewPiece.regen({(MAP_W + 1), (MAP_H + 1)});
    }

    void move(Time dt) {
        moveTimer += dt.s();
        if (moveTimer < timeBetweenMoves) return;
        moveTimer = 0.f;

        bool moved = currentPiece.move(0, -1);
        // TODO find a better way to check "lock"
        if (!moved) lock_piece();
    }

    void force_drop(Piece* p) {
        for (int i = 0; i < MAP_H; i++) p->move(0, -1);
    }

    void update_ghost() {
        ghostPiece = currentPiece;
        force_drop(&ghostPiece);
    }

    bool is_row_full(int row, int sum = 0) {
        for (int i = 0; i < MAP_W; i++) {
            auto col = grid[row * MAP_W + i].color;
            if (col != GRAY && col != BLACK) sum++;
        }
        return sum == MAP_W - 2;
    }

    void clear_row(int row) {
        for (int i = 0; i < MAP_W; i++) {
            if (i == 0 || i == MAP_W - 1) continue;  // skip walls
            grid[row * MAP_W + i].color = BLACK;
        }
    }

    void clear_full_rows() {
        for (int j = 0; j < MAP_H; j++) {
            if (!is_row_full(j)) continue;
            clear_row(j);
            // move everything above down
            for (int k = j; k < MAP_H; k++) {
                for (int x = 1; x < MAP_W - 1; x++) {
                    set_grid_color(x, k, grid[(k + 1) * MAP_W + x].color);
                }
            }
            // replace top row with empty tiles
            clear_row(MAP_H);
        }
    }

    void render() {
        Renderer::begin(cameraController->camera);
        {
            for (auto t : grid) t.render();
            previewPiece.render();
            currentPiece.render();
            ghostPiece.render(true);
        }
        Renderer::end();
    }

    virtual void onUpdate(Time dt) override {
        cameraController->onUpdate(dt);
        clear_full_rows();
        move(dt);
        update_ghost();
        render();
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        using namespace Key;
        int keycode = event.keycode;
        // TODO should space be rotate and up drop?
        if (keycode == KeyCode::Up) currentPiece.rotate();
        if (keycode == KeyCode::Left) currentPiece.move(-1, 0);
        if (keycode == KeyCode::Right) currentPiece.move(1, 0);
        if (keycode == KeyCode::Down) currentPiece.move(0, -1);
        if (keycode == KeyCode::Space) {
            force_drop(&currentPiece);
            lock_piece();
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
        .title = "tetrominoes",
        .clearEnabled = true,
        .escClosesWindow = true,
        .initResourcesFolder = "../resources",
    });

    Layer* demo = new DemoLayer();
    App::get().pushLayer(demo);

    App::get().run();

    return 0;
}
