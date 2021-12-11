

#include "../engine/app.h"
#include "../engine/input.h"
#include "../engine/pch.hpp"
#include "custom_fmt.h"
#include "entities.h"
#include "job.h"
#include "vecutil.h"

// TODO at some point we should have some way to send this to app
constexpr int WIN_W = 1920;
constexpr int WIN_H = 1080;
constexpr float WIN_RATIO = (WIN_W * 1.f) / WIN_H;
constexpr bool IS_DEBUG = true;

static std::shared_ptr<OrthoCameraController> cameraController;

// Requires access to the camera and entitites
#include "debug_layers.h"

struct SuperLayer : public Layer {
    SuperLayer() : Layer("Supermarket") {
        cameraController.reset(new OrthoCameraController(WIN_RATIO, true));

        Renderer::addTexture("./resources/box.png", 2.f);
        Renderer::addTexture("./resources/shelf.png", 2.f);
        Renderer::addTexture("./resources/face.png");
        Renderer::addTexture("./resources/screen.png");

        ////////////////////////////////////////////////////////

        auto billy = std::make_shared<Billboard>(
            glm::vec2{0.f, 0.f}, glm::vec2{1.f, 1.f}, 45.f,
            glm::vec4{0.0f, 1.0f, 1.0f, 1.0f}, "face");
        entities.push_back(billy);

        auto shelf = std::make_shared<Shelf>(
            glm::vec2{1.f, 1.f}, glm::vec2{1.f, 1.f}, 0.f,
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "box");
        shelf->contents.addItem(0, 1);
        shelf->contents.addItem(1, 6);
        shelf->contents.addItem(2, 7);
        shelf->contents.addItem(3, 9);
        entities.push_back(shelf);

        auto shelf2 = std::make_shared<Shelf>(
            glm::vec2{1.f, -3.f}, glm::vec2{1.f, 1.f}, 0.f,
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "box");
        entities.push_back(shelf2);

        // Job j = {
        // .type = JobType::Fill,
        // .startPosition = shelf->position,
        // .endPosition = shelf2->position,
        // .itemID = 0,
        // .itemAmount = 1,
        // };
        // JobQueue::addJob(JobType::Fill, std::make_shared<Job>(j));

        for (int i = 0; i < 1; i++) {
            auto emp = Employee();
            // TODO eventually get a texture
            // and fix colored textures
            emp.textureName = "__INVALID__";
            emp.color = gen_rand_vec4(0.3f, 1.0f);
            emp.color.w = 1.f;
            entities.push_back(std::make_shared<Employee>(emp));
        }
    }

    virtual ~SuperLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    virtual void onUpdate(Time dt) override {
        log_trace(fmt::format("{:.2}s ({:.2} ms) ", dt.s(), dt.ms()));
        prof(__PROFILE_FUNC__);

        cameraController->onUpdate(dt);
        for (auto& entity : entities) {
            entity->onUpdate(dt);
        }

        Renderer::begin(cameraController->camera);

        for (auto& entity : entities) {
            entity->render();
        }

        Renderer::end();

        // if (JobQueue::numOfJobsWithType(JobType::IdleWalk) < 5) {
        // JobQueue::addJob(
        // JobType::IdleWalk,
        // std::make_shared<Job>(
        // Job({.type = JobType::IdleWalk,
        // .startPosition = glm::circularRand<float>(5.f),
        // .endPosition = glm::circularRand<float>(5.f)})));
        // }
        // Cleanup all completed jobs
        JobQueue::cleanup();
    }

    bool onMouseButtonPressed(Mouse::MouseButtonPressedEvent& e) {
        auto mouse = Input::getMousePosition();
        glm::vec4 viewport = {0, 0, WIN_W, WIN_H};
        glm::vec3 mouseInWorld =
            glm::unProject(glm::vec3{mouse.x, WIN_H - mouse.y, 0.f},
                           cameraController->camera.view,
                           cameraController->camera.projection, viewport);

        // TODO allow people to remap their mouse buttons?
        if (e.GetMouseButton() == Mouse::MouseCode::ButtonLeft) {
            log_info(fmt::format("{} {}", mouse, mouseInWorld));
            JobQueue::addJob(
                JobType::DirectedWalk,
                std::make_shared<Job>(Job({.type = JobType::DirectedWalk,
                                           .endPosition = mouseInWorld})));
        }
        return false;
    }

    virtual void onEvent(Event& event) override {
        log_trace(event.toString());
        cameraController->onEvent(event);
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(std::bind(
            &SuperLayer::onMouseButtonPressed, this, std::placeholders::_1));
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    app.reset(App::create({
        .width = 1920,
        .height = 1080,
        .title = "SuperMarket",
        .clearEnabled = true,
    }));

    Layer* super = new SuperLayer();
    App::get().pushLayer(super);

    Layer* profile = new ProfileLayer();
    App::get().pushLayer(profile);

    Layer* entityDebug = new EntityDebugLayer();
    App::get().pushLayer(entityDebug);

    App::get().run();
    return 0;
}
