
#pragma once

#include "../engine/camera.h"
#include "../engine/layer.h"
#include "../engine/pch.hpp"
#include "../engine/ui.h"
//
#include "entities.h"

struct MenuLayer : public Layer {
    float value = 0.08f;
    std::string content = "";
    const int TYPABLE_START = 32;
    const int TYPABLE_END = 126;

    MenuLayer() : Layer("Supermarket") {
        Menu::get().state = Menu::State::Root;

        menuCameraController.reset(
            new OrthoCameraController(WIN_RATIO, true, 10.f, 0.f, 0.f));
        menuCameraController->camera.setPosition(glm::vec3{15.f, 0.f, 0.f});

        Renderer::addTexture("./resources/letters.png");
        int a = 0;
        int b = 0;
        for (int i = TYPABLE_START; i < TYPABLE_END; i++) {
            Renderer::addSubtexture("letters",                //
                                    std::string(1, (char)i),  //
                                    a, b, 16.f, 16.f);
            a++;
            if (a == 20) {
                b++;
                a = 0;
            }
        }
        IUI::init_context();
    }

    virtual ~MenuLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    bool onCharPressed(CharPressedEvent& event) {
        IUI::get()->keychar = static_cast<Key::KeyCode>(event.charcode);
        return false;
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        if (event.keycode == Key::mapping["Esc"]) {
            App::get().running = false;
            return true;
        }
        if (IUI::get()->widgetKeys.count(
                static_cast<Key::KeyCode>(event.keycode)) == 1) {
            IUI::get()->key = static_cast<Key::KeyCode>(event.keycode);
        }
        if (event.keycode == Key::mapping["Widget Mod"]) {
            IUI::get()->mod = static_cast<Key::KeyCode>(event.keycode);
        }
        if (IUI::get()->textfieldMod.count(
                static_cast<Key::KeyCode>(event.keycode)) == 1) {
            IUI::get()->modchar = static_cast<Key::KeyCode>(event.keycode);
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
        ui_test(dt);
        Renderer::end();
    }

    void ui_test(Time dt) {
        using namespace IUI;
        int item = 0;
        begin();
        {
            if (button(uuid({0, item++, 0}), glm::vec2{0.f, 0.f},
                       glm::vec2{2.f, 1.f})) {
                log_info("clicked button");
            }
            if (button(uuid({0, item++, 0}), glm::vec2{3.f, 0.f},
                       glm::vec2{2.f, 1.f})) {
                log_info("clicked button 2");
            }
            if (button(uuid({0, item++, 0}), glm::vec2{6.f, 0.f},
                       glm::vec2{2.f, 1.f})) {
                log_info("clicked button 3");
            }

            if (slider(uuid({0, item++, 0}), glm::vec2{9.f, 0.f},
                       glm::vec2{1.f, 3.f}, &value, 0.08f, 0.95f)) {
                // log_info("idk moved slider? ");
            }

            auto upperCaseConfig =
                WidgetConfig({.text = "THE FIVE BOXING WIZARDS JUMP QUICKLY",
                              .color = glm::vec4{1.0, 0.8f, 0.5f, 1.0f},
                              .position = glm::vec2{0.f, -4.f},
                              .size = glm::vec2{1.f, 1.f}});

            auto lowerCaseConfig =
                WidgetConfig({.text = "the five boxing wizards jump quickly",
                              .color = glm::vec4{0.8, 0.3f, 0.7f, 1.0f},
                              .position = glm::vec2{0.f, -6.f},
                              .size = glm::vec2{1.f, 1.f}});

            auto numbersConfig =
                WidgetConfig({.text = "0123456789",
                              .color = glm::vec4{0.7, 0.5f, 0.8f, 1.0f},
                              .position = glm::vec2{0.f, -8.f},
                              .size = glm::vec2{1.f, 1.f}});

            auto extrasConfig =
                WidgetConfig({.text = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",
                              .color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
                              .position = glm::vec2{0.f, -10.f},
                              .size = glm::vec2{1.f, 1.f}});

            text(uuid({0, item++, 0}), upperCaseConfig);
            text(uuid({0, item++, 0}), lowerCaseConfig);
            text(uuid({0, item++, 0}), numbersConfig);
            text(uuid({0, item++, 0}), extrasConfig);

            if (textfield(uuid({0, item++, 0}), glm::vec2{2.f, 2.f},
                          glm::vec2{3.f, 1.f}, content)) {
                log_info("{}", content);
            }
        }

        end();
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
        log_warn(event.toString().c_str());
        if (Menu::get().state == Menu::State::Game) return;

        menuCameraController->onEvent(event);
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(std::bind(
            &MenuLayer::onMouseButtonPressed, this, std::placeholders::_1));
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&MenuLayer::onKeyPressed, this, std::placeholders::_1));
        dispatcher.dispatch<CharPressedEvent>(
            std::bind(&MenuLayer::onCharPressed, this, std::placeholders::_1));
    }
};
