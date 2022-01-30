
#include "../../engine/app.h"
#include "../../engine/camera.h"
#include "../../engine/entity.h"
#include "../../engine/layer.h"
#include "../../engine/pch.hpp"

constexpr int WIN_W = 1920;
constexpr int WIN_H = 1080;
constexpr float WIN_RATIO = WIN_W * 1.f / WIN_H;
int leftPoints;
int rightPoints;

struct CanReset : public Entity {
    virtual void reset() {}
};

struct Paddle : public CanReset {
    void reset() override { position.y = WIN_H / 2.f; }
    virtual const char* typeString() const override { return "Paddle"; };

    bool isLeft;
    float speed;

    Paddle(bool left, glm::vec2 pos) : CanReset() {
        position = pos;
        size = {50.f, 300.f};
        isLeft = left;
        speed = 900.f;
    }

    virtual void onUpdate(Time dt) override {
        using namespace Key;
        bool up = Input::isKeyPressed(isLeft ? KeyCode::W : KeyCode::Up);
        bool down = Input::isKeyPressed(isLeft ? KeyCode::S : KeyCode::Down);
        int dir = (!up && !down) ? 0 : down ? -1 : 1;
        position.y = position.y + (dir * speed * dt.s());
        position.y = fmin(WIN_H, fmax(0, position.y));
    }
};

struct Ball : public CanReset {
    void reset() override {
        position = glm::vec2{WIN_W / 2.f, WIN_H / 2.f};
        vel *= 0.f;
    }
    virtual const char* typeString() const override { return "Ball"; };

    glm::vec2 vel;
    float mxspeed = 1000.f;
    float speed = 1000.f;

    Ball(glm::vec2 pos) : CanReset() {
        position = pos;
        size = glm::vec2{20.f};
    }

    virtual void onUpdate(Time dt) override {
        if (glm::length(vel) > mxspeed) {
            vel = glm::normalize(vel) * mxspeed;
        }
        position += vel * dt.s();
        if (position.x < -100) {
            rightPoints += 1;
            reset();
        }
        if (position.x > WIN_W + 100) {
            leftPoints += 1;
            reset();
        }
        if (position.y < 0 || position.y > WIN_H) vel.y *= -1;
    }

    void collide(Paddle paddle) {
        if (aabb(position, size, paddle.position, paddle.size)) vel.x *= -1;
    }

    void go() {
        if (glm::length(vel) > 1.f) return;
        float angle = ((float)(rand()) / (float)(RAND_MAX)) * 6.28;
        vel = {speed * cos(angle), speed * sin(angle)};
    }
};

struct PongLayer : public Layer {
    std::shared_ptr<OrthoCameraController> pongCameraController;
    std::shared_ptr<Ball> ball;

    PongLayer() : Layer("Pong") {
        pongCameraController.reset(
            new OrthoCameraController(WIN_RATIO, 800.f, 0.f, 0.f));
        pongCameraController->camera.setPosition(
            glm::vec3{WIN_W / 2.f, WIN_H / 2.f, 0.f});

        pongCameraController->camera.setViewport(glm::vec4{0, 0, WIN_W, WIN_H});

        EntityHelper::addEntity(
            std::make_shared<Paddle>(true, glm::vec2{10, WIN_H / 2}));
        EntityHelper::addEntity(
            std::make_shared<Paddle>(false, glm::vec2{WIN_W, WIN_H / 2}));

        ball.reset(new Ball(glm::vec2{WIN_W / 2, WIN_H / 2}));
        EntityHelper::addEntity(ball);

        reset();
    }

    void reset() {
        EntityHelper::forEachV<CanReset>([](auto e) { e->reset(); });
        leftPoints = 0, rightPoints = 0;
    }

    virtual ~PongLayer() {}

    virtual void onUpdate(Time dt) override {
        pongCameraController->onUpdate(dt);
        Renderer::begin(pongCameraController->camera);
        EntityHelper::forEachEntity([dt](auto e) {
            e->onUpdate(dt);
            e->render();
            return EntityHelper::ForEachFlow::None;
        });
        EntityHelper::forEachV<Ball>([](auto b) {
            EntityHelper::forEachV<Paddle>([b](auto p) { b->collide(*p); });
        });
        Renderer::end();
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        if (event.keycode == Key::KeyCode::D0) reset();
        if (event.keycode == Key::KeyCode::Space) ball->go();
        return false;
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&PongLayer::onKeyPressed, this, std::placeholders::_1));
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    App::create({
        .width = WIN_W,
        .height = WIN_H,
        .title = "pong",
        .clearEnabled = true,
        .escClosesWindow = true,
        .initResourcesFolder = "../resources",
    });

    Layer* pong = new PongLayer();
    App::get().pushLayer(pong);

    App::get().run();

    return 0;
}
