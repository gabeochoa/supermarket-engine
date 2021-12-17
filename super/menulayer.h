
#pragma once

#include "../engine/camera.h"
#include "../engine/layer.h"
#include "../engine/pch.hpp"
#include "../engine/ui.h"
//
#include "entities.h"

struct MenuLayer : public Layer {
    MenuLayer() : Layer("Supermarket") {
        Menu::get().state = Menu::State::Root;

        menuCameraController.reset(
            new OrthoCameraController(WIN_RATIO, true, 10.f, 5.f, 0.f));
        menuCameraController->camera.setPosition(glm::vec3{15.f, 0.f, 0.f});

        Renderer::addTexture("./resources/letters.png");
        auto s = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
        int a = 0;
        int b = 0;
        for (int i = 0; i < 27; i++) {
            Renderer::addSubtexture("letters",             //
                                    std::string(1, s[i]),  //
                                    a, b, 16.f, 16.f);
            a++;
            if (a == 10) {
                b++;
                a = 0;
            }
        }
    }

    virtual ~MenuLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    bool onKeyPressed(KeyPressedEvent& event) {
        if (event.keycode == Key::mapping["Esc"]) {
            App::get().running = false;
            return true;
        }
        return false;
    }

    virtual void onUpdate(Time dt) override {
        log_trace("{:.2}s ({:.2} ms) ", dt.s(), dt.ms());
        prof(__PROFILE_FUNC__);

        if (Menu::get().state == Menu::State::Game) {
            return;
        }
        menuCameraController->onUpdate(dt);

        gltInit();
        gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        GLTtext* text = gltCreateText();
        gltSetText(text, stateToString(Menu::get().state));
        gltDrawText2D(text, 150, 150, 5.f);
        gltEndDraw();
        gltDeleteText(text);
        gltTerminate();

        Renderer::begin(menuCameraController->camera);
        ui_test();
        Renderer::end();
    }

    void ui_test() {
        auto textConfig =
            IUI::TextConfig({.text = "THE LAZY BROWN DOG JUMPS OVER THE LOG",
                             .color = glm::vec4{1.0, 0.8f, 0.5f, 1.0f},
                             .position = glm::vec2{0.f, 0.f},
                             .size = glm::vec2{2.f, 2.f}});
        auto buttonConfig = IUI::ButtonConfig({.textConfig = textConfig,
                                               .position = glm::vec2{2.f, 1.f},
                                               .color = glm::vec4{1.f},
                                               .size = glm::vec2{2.f, 1.f}});

        if (IUI::button(*IUI::get(),
                        IUI::uuid({.owner = 0, .item = 0, .index = 0}),
                        buttonConfig)) {
            // log_info("button was clicked ");
        }
    }

    bool onMouseButtonPressed(Mouse::MouseButtonPressedEvent& e) {
        (void)e;
        // auto mouse = Input::getMousePosition();
        // glm::vec4 viewport = {0, 0, WIN_W, WIN_H};
        // glm::vec3 mouseInWorld =
        // glm::unProject(glm::vec3{mouse.x, WIN_H - mouse.y, 0.f},
        // cameraController->camera.view,
        // cameraController->camera.projection, viewport);
        if (e.GetMouseButton() == Mouse::MouseCode::ButtonRight) {
            Menu::get().state = Menu::State::Game;
        }
        return false;
    }

    virtual void onEvent(Event& event) override {
        // log_warn(event.toString().c_str());
        if (Menu::get().state == Menu::State::Game) return;

        menuCameraController->onEvent(event);
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(std::bind(
            &MenuLayer::onMouseButtonPressed, this, std::placeholders::_1));
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&MenuLayer::onKeyPressed, this, std::placeholders::_1));
    }
};
