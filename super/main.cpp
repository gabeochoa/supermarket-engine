

#include "../engine/app.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"

// TODO at some point we should have some way to send this to app
constexpr int WIN_W = 1920;
constexpr int WIN_H = 1080;
constexpr float WIN_RATIO = (WIN_W * 1.f) / WIN_H;

struct Entity {
    glm::vec2 position;
    glm::vec2 size;
    float angle;
    glm::vec4 color;
    std::string textureName;

    Entity()
        : position({0.f, 0.f}),
          size({1.f, 1.f}),
          angle(0.f),
          color({1.f, 1.f, 1.f, 1.f}),
          textureName("white") {}

    Entity(const glm::vec2& position_, const glm::vec2& size_, float angle_,
           const glm::vec4& color_, const std::string& textureName_)
        : position(position_),
          size(size_),
          angle(angle_),
          color(color_),
          textureName(textureName_) {}

    virtual ~Entity() {}

    virtual void onUpdate(Time dt) {
        (void)dt;
        if (angle >= 360) {
            angle -= 360;
        }
        if (angle < 0) {
            angle += 360;
        }
    }

    virtual void render() {
        // computing angle transforms are expensive so
        // if the angle is under thresh, just render it square
        if (angle <= 5.f) {
            Renderer::drawQuad(position, size, color, textureName);
        } else {
            Renderer::drawQuadRotated(position, size, glm::radians(angle),
                                      color, textureName);
        }
    }
};

struct Billboard : public Entity {
    // Billboard is a textured ent that never moves
    Billboard(const glm::vec2& position, const glm::vec2& size, float angle,
              const glm::vec4& color,
              // TODO somehow i broke colored textures...
              // so for now we will use a definite undefinied tex
              // and it will trigger the flat shader
              const std::string& textureName = "__INVALID__")
        : Entity(position, size, angle, color, textureName) {}

    virtual ~Billboard() {}
};

struct SuperLayer : public Layer {
    std::vector<std::shared_ptr<Entity>> entities;

    OrthoCameraController cameraController;

    SuperLayer() : Layer("Supermarket"), cameraController(WIN_RATIO, true) {
        auto billy = std::make_shared<Billboard>(
            glm::vec2{0.f, 0.f}, glm::vec2{1.f, 1.f}, 45.f,
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "face");
        entities.push_back(billy);

        billy = std::make_shared<Billboard>(glm::vec2{0.5f, 1.f},
                                            glm::vec2{0.5f, 0.5f}, 0.f,
                                            glm::vec4{0.8f, 0.3f, 0.0f, 1.0f});
        entities.push_back(billy);

        billy = std::make_shared<Billboard>(glm::vec2{-0.5f, -1.f},
                                            glm::vec2{1.1f, 1.1f}, 0.f,
                                            glm::vec4{0.2f, 0.7f, 0.0f, 1.0f});
        entities.push_back(billy);
    }

    virtual ~SuperLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    virtual void onUpdate(Time dt) override {
        log_trace(fmt::format("{:.2}s ({:.2} ms) ", dt.s(), dt.ms()));
        prof(__PROFILE_FUNC__);

        cameraController.onUpdate(dt);
        for (auto& entity : entities) {
            entity->onUpdate(dt);
        }

        Renderer::clear(/* color */ {0.1f, 0.1f, 0.1f, 1.0f});
        Renderer::begin(cameraController.camera);

        for (auto& entity : entities) {
            entity->render();
        }

        Renderer::end();
    }

    virtual void onEvent(Event& event) override {
        log_trace(event.toString());
        cameraController.onEvent(event);
    }
};

struct ProfileLayer : public Layer {
    bool showFilenames;
    ProfileLayer() : Layer("Profiling"), showFilenames(false) {}

    virtual ~ProfileLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    GLTtext* drawText(const std::string& content, int x, int y, float scale) {
        GLTtext* text = gltCreateText();
        gltSetText(text, content.c_str());
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        gltDrawText2D(text, x, y, scale);
        return text;
    }

    virtual void onUpdate(Time dt) override {
        (void)dt;
        if (isMinimized) {
            return;
        }

        prof(__PROFILE_FUNC__);

        gltInit();
        int y = 10;
        float scale = 1.f;
        std::vector<GLTtext*> texts;
        gltBeginDraw();

        std::vector<SamplePair> pairs;
        for (const auto& x : _acc) {
            pairs.push_back(x);
        }

        sort(pairs.begin(), pairs.end(),
             [](const SamplePair& a, const SamplePair& b) {
                 return a.second.average() > b.second.average();
             });

        for (const auto& x : pairs) {
            auto name = x.first;
            auto stats = x.second;
            auto filename = showFilenames ? stats.filename : "";
            std::string t = fmt::format("{}{}: avg: {:.2f}ns", filename, name,
                                        stats.average());
            texts.push_back(drawText(t, 10, y, scale));
            y += 30;
        }

        texts.push_back(
            drawText(fmt::format("Press delete to toggle filenames {}",
                                 showFilenames ? "off" : "on"),
                     0, y, scale));
        y += 30;

        gltEndDraw();
        for (auto text : texts) gltDeleteText(text);
        gltTerminate();
    }

    bool onKeyPressed(KeyPressedEvent event) {
        if (event.keycode == Key::mapping["Open Profiler"]) {
            isMinimized = !isMinimized;
        }

        if (event.keycode == Key::mapping["Profiler Hide Filenames"]) {
            showFilenames = !showFilenames;
        }

        return false;
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(std::bind(
            &ProfileLayer::onKeyPressed, this, std::placeholders::_1));
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    App::get();

    Layer* profile = new ProfileLayer();
    App::get().pushLayer(profile);

    Layer* super = new SuperLayer();
    App::get().pushLayer(super);

    App::get().run();
    return 0;
}
