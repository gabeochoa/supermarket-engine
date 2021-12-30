
#pragma once

#include "../engine/camera.h"
#include "../engine/layer.h"
#include "../engine/pch.hpp"
#include "../engine/renderer.h"
//
#include "global.h"
//
#include "entities.h"
#include "job.h"
#include "menu.h"

struct SuperLayer : public Layer {
    // TODO should we add some system for "named" entities
    // allowing to store this globally and hide it from the layer
    std::shared_ptr<DragArea> dragArea;

    SuperLayer() : Layer("Supermarket") {
        isMinimized = true;

        cameraController.reset(new OrthoCameraController(WIN_RATIO, true));
        cameraController->camera.setViewport(glm::vec4{0, 0, WIN_W, WIN_H});

        Renderer::addTexture("./resources/face.png");
        Renderer::addTexture("./resources/box.png");
        Renderer::addTexture("./resources/shelf.png");
        Renderer::addTexture("./resources/screen.png");

        // 918 × 203 pixels
        // 16 x 16
        // margin 1
        float playerSprite = 16.f;
        Renderer::addTexture("./resources/character_tilesheet.png");
        Renderer::addSubtexture("character_tilesheet", "player", 0, 0,
                                playerSprite, playerSprite);
        Renderer::addSubtexture("character_tilesheet", "player2", 0, 1,
                                playerSprite, playerSprite);
        Renderer::addSubtexture("character_tilesheet", "player3", 1, 1,
                                playerSprite, playerSprite);

        Renderer::addTexture("./resources/item_sheet.png");
        Renderer::addSubtexture("item_sheet", "egg", 0, 0, 16.f, 16.f);
        Renderer::addSubtexture("item_sheet", "milk", 1, 0, 16.f, 16.f);
        Renderer::addSubtexture("item_sheet", "peanutbutter", 2, 0, 16.f, 16.f);
        Renderer::addSubtexture("item_sheet", "pizza", 3, 0, 16.f, 16.f);

        ////////////////////////////////////////////////////////
        auto storage = std::make_shared<Storage>(
            glm::vec2{1.f, 1.f}, glm::vec2{1.f, 1.f}, 0.f,
            glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "box");
        storage->contents.addItem(0, 1);
        storage->contents.addItem(1, 6);
        storage->contents.addItem(2, 7);
        storage->contents.addItem(3, 9);
        entities.push_back(storage);

        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 10; j += 2) {
                auto shelf2 = std::make_shared<Shelf>(
                    glm::vec2{1.f + i, -3.f + j},  //
                    glm::vec2{1.f, 1.f}, 0.f,      //
                    glm::vec4{1.0f, 1.0f, 1.0f, 1.0f}, "shelf");
                entities.push_back(shelf2);
            }
        }

        const int num_people_sprites = 3;
        std::array<std::string, num_people_sprites> peopleSprites = {
            "player",
            "player2",
            "player3",
        };

        for (int i = 0; i < 5; i++) {
            auto emp = Employee();
            emp.color = gen_rand_vec4(0.3f, 1.0f);
            emp.color.w = 1.f;
            emp.size = {0.6f, 0.6f};
            emp.textureName = peopleSprites[i % num_people_sprites];
            entities.push_back(std::make_shared<Employee>(emp));
        }

        dragArea.reset(new DragArea(glm::vec2{0.f}, glm::vec2{0.f}, 0.f,
                                    glm::vec4{0.75f}));
    }

    virtual ~SuperLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    void child_updates(Time dt) {
        cameraController->onUpdate(dt);
        for (auto& entity : entities) {
            entity->onUpdate(dt);
        }
        dragArea->onUpdate(dt);
    }

    void render() {
        Renderer::begin(cameraController->camera);
        for (auto& entity : entities) {
            entity->render();
        }
        dragArea->render();
        Renderer::end();
    }

    void fillJobQueue() {
        if (JobQueue::numOfJobsWithType(JobType::IdleWalk) < 5) {
            JobQueue::addJob(
                JobType::IdleWalk,
                std::make_shared<Job>(
                    Job({.type = JobType::IdleWalk,
                         .endPosition = glm::circularRand<float>(5.f)})));
        }

        for (auto& entity : entities) {
            auto storage = dynamic_pointer_cast<Storage>(entity);
            if (storage) {
                // TODO for now just keep queue jobs until we are empty
                if (!storage->contents.empty() &&
                    JobQueue::numOfJobsWithType(JobType::Fill) <
                        (int)storage->contents.size()) {
                    // TODO getting random shelf probably not the best
                    // idea.. .
                    Job j = {
                        .type = JobType::Fill,
                        .startPosition = storage->position,
                        .endPosition =
                            Storage::getRandomStorage<Shelf>()->position,
                        .itemID = storage->contents.rbegin()->first,
                        .itemAmount = storage->contents.rbegin()->second,
                    };
                    JobQueue::addJob(JobType::Fill, std::make_shared<Job>(j));
                }
            }
        }
    }

    glm::vec3 getMouseInWorld() {
        auto mouse = Input::getMousePosition();
        glm::vec4 viewport = {0, 0, WIN_W, WIN_H};
        return screenToWorld(glm::vec3{mouse.x, WIN_H - mouse.y, 0.f},
                             cameraController->camera.view,
                             cameraController->camera.projection, viewport);
    }

    bool onMouseButtonPressed(Mouse::MouseButtonPressedEvent& e) {
        glm::vec3 mouseInWorld = getMouseInWorld();

        // TODO allow people to remap their mouse buttons?
        if (e.GetMouseButton() == Mouse::MouseCode::ButtonLeft) {
            dragArea->mouseDragStart = mouseInWorld;
            dragArea->mouseDragEnd = mouseInWorld;
        }
        if (e.GetMouseButton() == Mouse::MouseCode::ButtonRight) {
            JobQueue::addJob(
                JobType::DirectedWalk,
                std::make_shared<Job>(Job({.type = JobType::DirectedWalk,
                                           .endPosition = mouseInWorld})));
        }
        return false;
    }
    bool onMouseMoved(Mouse::MouseMovedEvent& e) {
        glm::vec3 mouseInWorld = getMouseInWorld();

        if (Input::isMouseButtonPressed(Mouse::MouseCode::ButtonLeft)) {
            dragArea->isMouseDragging = true;
            dragArea->mouseDragEnd = mouseInWorld;
        }
        return false;
    }

    bool onMouseButtonReleased(Mouse::MouseButtonReleasedEvent& e) {
        glm::vec3 mouseInWorld = getMouseInWorld();

        if (e.GetMouseButton() == Mouse::MouseCode::ButtonLeft) {
            dragArea->mouseDragEnd = mouseInWorld;
            // TODO should this live in mouseMoved?
            dragArea->isMouseDragging = false;
        }
        return false;
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        if (event.keycode == Key::mapping["Esc"]) {
            Menu::get().state = Menu::State::Root;
            return true;
        }
        return false;
    }

    virtual void onUpdate(Time dt) override {
        if (Menu::get().state != Menu::State::Game) return;

        log_trace("{:.2}s ({:.2} ms) ", dt.s(), dt.ms());
        prof(__PROFILE_FUNC__);  //

        Renderer::stats.reset();
        Renderer::stats.begin();
        child_updates(dt);    // move things around
        render();             // draw everything
        fillJobQueue();       // add more jobs if needed
        JobQueue::cleanup();  // Cleanup all completed jobs
        Renderer::stats.end();
    }

    virtual void onEvent(Event& event) override {
        if (Menu::get().state != Menu::State::Game) return;
        // log_warn(event.toString().c_str());
        cameraController->onEvent(event);
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(std::bind(
            &SuperLayer::onMouseButtonPressed, this, std::placeholders::_1));
        dispatcher.dispatch<Mouse::MouseButtonReleasedEvent>(std::bind(
            &SuperLayer::onMouseButtonReleased, this, std::placeholders::_1));
        dispatcher.dispatch<Mouse::MouseMovedEvent>(
            std::bind(&SuperLayer::onMouseMoved, this, std::placeholders::_1));
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&SuperLayer::onKeyPressed, this, std::placeholders::_1));
    }
};
