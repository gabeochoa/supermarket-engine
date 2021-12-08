

#include "../engine/app.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"

// TODO at some point we should have some way to send this to app
constexpr int WIN_W = 1920;
constexpr int WIN_H = 1080;
constexpr float WIN_RATIO = (WIN_W * 1.f) / WIN_H;

struct SuperLayer : public Layer {
    TextureLibrary textureLibrary;

    OrthoCameraController cameraController;

    SuperLayer() : Layer("Supermarket"), cameraController(WIN_RATIO, true) {
        textureLibrary.load("./resources/face.png");
        textureLibrary.load("./resources/screen.png", 1);
    }

    virtual ~SuperLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    virtual void onUpdate(Time dt) override {
        cameraController.onUpdate(dt);

        log_trace(fmt::format("{:.2}s ({:.2} ms) ", dt.s(), dt.ms()));

        Renderer::clear(/* color */ {0.1f, 0.1f, 0.1f, 1.0f});
        Renderer::begin(cameraController.camera);

        Renderer::drawQuad(glm::vec2{0.f, 0.f}, glm::vec2{1.f, 1.f},
                           glm::vec4{0.8f, 0.3f, 0.2f, 1.0f},
                           textureLibrary.get("face"));

        Renderer::drawQuad(glm::vec2{0.5f, 1.f}, glm::vec2{0.8f, 0.8f},
                           glm::vec4{0.5f, 0.4f, 0.2f, 1.0f});

        Renderer::end();
    }

    virtual void onEvent(Event& event) override {
        log_trace(event.toString());
        cameraController.onEvent(event);
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    App::get();

    Layer* super = new SuperLayer();
    App::get().pushLayer(super);

    App::get().run();
    return 0;
}
