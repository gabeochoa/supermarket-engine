
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

const glm::vec4 BLACK{0.2f};
const glm::vec4 GRAY{0.5, 0.5f, 0.5f, 1.f};
const glm::vec4 WHITE{1.f};

struct Tile {
    glm::vec2 position;
    glm::vec4 color;

    void render() {
        auto size = glm::vec2{TILESIZE};
        auto pos = position + (size / 2.f);
        pos += (size * 0.1f);
        size *= 0.8;
        Renderer::drawQuad(pos, size, color, "white");

        auto darker =
            glm::vec4{color.r / 2, color.g / 2, color.b / 2, color.a / 2};
        Renderer::drawQuad(pos, size, darker, "white");
    }
};
std::array<Tile, MAP_H * MAP_W> grid;

struct DemoLayer : public Layer {
    std::shared_ptr<OrthoCameraController> cameraController;

    struct Piece {
        glm::vec2 position;
        int type = 0;
        std::array<int, 16> piece;
        glm::vec4 bounds;
        int angle = 0;

        Piece() {}

        Piece(const Piece& other) {
            this->position = other.position;
            this->type = other.type;
            this->piece = other.piece;
            this->bounds = other.bounds;
            this->angle = other.angle;
        }

        Piece& operator=(const Piece& other) {
            this->position = other.position;
            this->type = other.type;
            this->piece = other.piece;
            this->bounds = other.bounds;
            this->angle = other.angle;
            return *this;
        }

        // pieces and rotations come from:
        // https://tetris.wiki/File:SRS-pieces.png
        const std::array<int, 4> tower = {{
            0b1111000000000000,
            0b0010001000100010,
            0b0000000011110000,
            0b0100010001000100,
        }};

        const std::array<int, 4> box = {{
            0b0110011000000000,
            0b0110011000000000,
            0b0110011000000000,
            0b0110011000000000,
        }};

        const std::array<int, 4> pyramid = {{
            0b0100111000000000,
            0b0100011001000000,
            0b0000111001000000,
            0b0100110001000000,
        }};

        const std::array<int, 4> leftlean = {{
            0b1100011000000000,
            0b0010011001000000,
            0b0000110001100000,
            0b0100110010000000,
        }};

        const std::array<int, 4> rightlean = {{
            0b0110110000000000,
            0b0100011000100000,
            0b0000011011000000,
            0b1000110001000000,
        }};

        const std::array<int, 4> leftknight = {{
            0b1000111000000000,
            0b0110010001000000,
            0b0000111000100000,
            0b0100010011000000,
        }};

        const std::array<int, 4> rightknight = {{
            0b0010111000000000,
            0b0110010001000000,
            0b0000111000100000,
            0b0100010011000000,
        }};

        std::array<int, 16> type_to_rotated_array(int t, int a) {
            if (t == 0) return bit_to_array(tower[a]);
            if (t == 1) return bit_to_array(box[a]);
            if (t == 2) return bit_to_array(pyramid[a]);
            if (t == 3) return bit_to_array(leftlean[a]);
            if (t == 4) return bit_to_array(rightlean[a]);
            if (t == 5) return bit_to_array(leftknight[a]);
            if (t == 6) return bit_to_array(rightknight[a]);
            log_warn("got a type idk about {}", t);
            return bit_to_array(tower[a]);
        }

        std::array<int, 16> bit_to_array(int b) {
            std::array<int, 16> tmp;
            int j = 0;
            for (int i = 15; i >= 0; i--) {
                tmp[j++] = (b >> i) & 1;
            }
            return tmp;
        }

        void regen() {
            type = rand() % 6;
            angle = 0;
            piece = type_to_rotated_array(type, angle);
            bounds = genBounds();
        }

        glm::vec4 genBounds() {
            glm::vec4 results{5, 5, -1, -1};
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (piece[j * 4 + i] == 1) {
                        results.x = fmin(results.x, i);
                        results.y = fmin(results.y, j);

                        results.z = fmax(results.z, i);
                        results.w = fmax(results.w, j);
                    }
                }
            }
            return results;
        }

        bool collidesWithMap(int x, int y, std::array<int, 16> p) {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (p[j * 4 + i] == 0) continue;
                    if (grid[(y + j) * MAP_W + (x + i)].color.a < 0.4f)
                        continue;
                    return true;
                }
            }
            return false;
        }

        std::array<std::pair<int, int>, 4> getTests() {
            if (type == 0) {
                std::array<std::array<std::pair<int, int>, 4>, 4> tests = {{
                    {{
                        {-2, +0},
                        {+1, +0},
                        {-2, -1},
                        {+1, +2},
                    }},
                    {{
                        {-1, +0},
                        {+2, +0},
                        {-1, +2},
                        {+2, -1},
                    }},
                    {{
                        {+2, +0},
                        {-1, +0},
                        {+2, +1},
                        {-1, -2},
                    }},
                    {{
                        {+1, +0},
                        {-2, +0},
                        {+1, -2},
                        {-2, +1},
                    }},
                }};
                return tests[angle];
            }
            std::array<std::array<std::pair<int, int>, 4>, 4> tests = {{
                {{
                    {-1, +0},
                    {-1, +1},
                    {-0, -2},
                    {-1, -2},
                }},
                {{
                    {+1, +0},
                    {+1, -1},
                    {+0, +2},
                    {+1, +2},
                }},
                {{
                    {+1, +0},
                    {+1, +1},
                    {+0, -2},
                    {+1, -2},
                }},
                {{
                    {-1, +0},
                    {-1, -1},
                    {+0, +2},
                    {-1, +2},
                }},
            }};
            return tests[angle];
        }

        void rotate() {
            // rotate the array
            std::array<int, 16> tmp =
                type_to_rotated_array(type, (angle + 1) % 4);

            // check if rotation fits,
            if (!collidesWithMap(position.x, position.y, tmp)) {
                // yep, lets change the angle and write to the piece
                angle = (angle + 1) % 4;
                piece = tmp;
                return;
            }

            // rotation didnt fit,
            // lets wall kick

            int x = position.x;
            int y = position.y;
            for (auto pair : getTests()) {
                if (collidesWithMap(x + pair.first, y + pair.second, tmp))
                    continue;
                // no collision we good
                position.x = x + pair.first;
                position.y = y + pair.second;
                angle = (angle + 1) % 4;
                piece = tmp;
                return;
            }
        }

        void move(int a, int b) {
            int nx = position.x + a;
            int ny = position.y + b;
            if (collidesWithMap(nx, ny, piece)) return;
            position.x = nx;
            position.y = ny;
        }

        glm::vec4 color() {
            if (type == 0) return glm::vec4{0.0, 0.2, 0.8, 1.f};
            if (type == 1) return glm::vec4{0.8, 0.3, 0.0, 1.f};
            if (type == 2) return glm::vec4{0.0, 0.8, 0.1, 1.f};
            if (type == 3) return glm::vec4{0.4, 0.0, 0.6, 1.f};
            if (type == 4) return glm::vec4{0.7, 0.7, 0.8, 1.f};
            if (type == 5) return glm::vec4{0.2, 0.2, 0.6, 1.f};
            if (type == 6) return glm::vec4{0.6, 0.4, 0.2, 1.f};
            return WHITE;
        }

        void render(bool ghost = false) {
            auto size = glm::vec2{TILESIZE};
            auto gridpos = position * (1.f * TILESIZE) + (TILESIZE * 0.1f);

            auto col = color();
            if (ghost) col.a = 0.3f;

            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (piece[j * 4 + i] == 1) {
                        Renderer::drawQuad(
                            (gridpos + (glm::vec2{i, j} * size)) + (size / 2.f),
                            size * 0.8f, col, "white");
                    }
                }
            }
        }
    };

    Piece currentPiece;
    Piece previewPiece;
    Piece ghostPiece;
    float moveTimer;

    DemoLayer() : Layer("") {
        init_camera();
        init_map();
    }

    virtual ~DemoLayer() {}

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
                auto color = BLACK;
                if (j == 0) color = GRAY;
                if (i == 0 || i == MAP_W - 1) color = GRAY;
                grid[j * MAP_W + i] = Tile({
                    .position = {i * TILESIZE, j * TILESIZE},
                    .color = color,
                });
            }
        }

        previewPiece.position = {(MAP_W + 1), (MAP_H + 1)};
        currentPiece.position = {(MAP_W / 2), (MAP_H - 1)};
        previewPiece.regen();
        currentPiece.regen();
    }

    void lock_piece() {
        int x = currentPiece.position.x;
        int y = currentPiece.position.y;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (currentPiece.piece[j * 4 + i] == 1) {
                    grid[(y + j) * MAP_W + (x + i)].color =
                        currentPiece.color();
                }
            }
        }
        currentPiece = previewPiece;
        currentPiece.position = {(MAP_W / 2), (MAP_H)};

        previewPiece.regen();
    }

    void move(Time dt) {
        moveTimer += dt.s();
        if (moveTimer < 0.5f) return;
        moveTimer = 0.f;

        glm::vec2 p = currentPiece.position;
        currentPiece.move(0, -1);
        // TODO find a better way to check "lock"
        if (currentPiece.position.x == p.x && currentPiece.position.y == p.y) {
            lock_piece();
        }
    }

    void update_ghost() {
        ghostPiece = currentPiece;
        for (int i = 0; i < MAP_H; i++) ghostPiece.move(0, -1);
    }

    void clear_rows() {
        for (int j = 0; j < MAP_H; j++) {
            int sum = 0;
            for (int i = 0; i < MAP_W; i++) {
                auto col = grid[j * MAP_W + i].color;
                if (col == GRAY || col == BLACK) continue;
                sum += 1;
            }
            if (sum == MAP_W - 2) {
                // clear row and move down
                for (int i = 1; i < MAP_W - 1; i++) {
                    grid[j * MAP_W + i].color = BLACK;
                }
                for (int k = j; k < MAP_H; k++) {
                    for (int i = 1; i < MAP_W - 1; i++) {
                        grid[k * MAP_W + i].color =
                            grid[(k + 1) * MAP_W + i].color;
                    }
                }

                for (int i = 1; i < MAP_W - 1; i++) {
                    grid[MAP_H * MAP_W + i].color = BLACK;
                }
            }
        }
    }

    virtual void onUpdate(Time dt) override {
        cameraController->onUpdate(dt);
        clear_rows();
        move(dt);
        update_ghost();

        Renderer::begin(cameraController->camera);
        for (auto t : grid) t.render();
        previewPiece.render();
        currentPiece.render();
        ghostPiece.render(true);
        Renderer::end();
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        using namespace Key;
        int keycode = event.keycode;
        if (keycode == KeyCode::Up) currentPiece.rotate();
        if (keycode == KeyCode::Left) currentPiece.move(-1, 0);
        if (keycode == KeyCode::Right) currentPiece.move(1, 0);
        if (keycode == KeyCode::Down) currentPiece.move(0, -1);
        if (keycode == KeyCode::Space) {
            for (int i = 0; i < MAP_H; i++) currentPiece.move(0, -1);
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
