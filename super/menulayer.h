
#pragma once

#include "../engine/camera.h"
#include "../engine/layer.h"
#include "../engine/pch.hpp"
#include "../engine/ui.h"
//
#include "entities.h"

struct MenuLayer : public Layer {
    float value = 0.05f;
    std::string content = "";
    const int TYPABLE_START = 32;
    const int TYPABLE_END = 123;

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
        if (event.keycode >= TYPABLE_START && event.keycode <= TYPABLE_END) {
            IUI::get()->keychar = static_cast<Key::KeyCode>(event.keycode);
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
        // before
        {
            get()->hotID = IUI::rootID;
            get()->lmouseDown =
                Input::isMouseButtonPressed(Mouse::MouseCode::ButtonLeft);
            get()->mousePosition =
                screenToWorld(glm::vec3{Input::getMousePosition(), 0.f},
                              menuCameraController->camera.view,        //
                              menuCameraController->camera.projection,  //
                              glm::vec4{0, 0, WIN_W, WIN_H});
        }

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
                   glm::vec2{1.f, 3.f}, &value, 0.05f, 0.95f)) {
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

        // after
        {
            if (get()->lmouseDown) {
                if (get()->activeID == IUI::rootID) {
                    get()->activeID = fakeID;
                }
            } else {
                get()->activeID = IUI::rootID;
            }
            get()->key = Key::KeyCode();
            get()->mod = Key::KeyCode();

            get()->keychar = Key::KeyCode();
            get()->modchar = Key::KeyCode();
        }

        /*

        auto textConfig =
            TextConfig({.text = "Resume Game",
                        .color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f},
                        .position = glm::vec2{0.f, 0.f},
                        .size = glm::vec2{0.25f, 0.25f}});
        auto buttonConfig =
            ButtonConfig({.textConfig = textConfig,
                          .position = glm::vec2{0.f, 0.f},
                          .color = glm::vec4{0.4f, 0.3f, 0.8f, 1.f},
                          .size = glm::vec2{4.f, 1.f}});

        if (button(get(), uuid({0, item++, 0}), buttonConfig)) {
            Menu::get().state = Menu::State::Game;
        }

        auto whiteC = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        auto ddsize = glm::vec2{0.25f};

        std::array<ToggleConfig, 2> toggleConfigs = {
            ToggleConfig({.position = glm::vec2{4.f, 0.f},
                          .color = glm::vec4{0.4f, 0.3f, 0.8f, 1.f},
                          .size = glm::vec2{4.f, 1.f}}),
            ToggleConfig({.position = glm::vec2{4.f, 0.f},
                          .color = glm::vec4{0.8f, 0.4f, 0.3f, 1.f},
                          .size = glm::vec2{4.f, 1.f}}),
        };
        ToggleState toggleState = ToggleState(false);
        auto t =
            toggle(get(), uuid({0, item++, 0}), toggleConfigs, toggleState);
        (void)t;
        */

        // std::vector<TextConfig> ddtextConfigs = std::vector<TextConfig>({
        // TextConfig({.text = "Option 1",
        // .color = whiteC,
        // .position = glm::vec2{0.f, 0.f},
        // .size = ddsize}),
        // TextConfig({.text = "Option 2",
        // .color = whiteC,
        // .position = glm::vec2{0.f, 0.f},
        // .size = ddsize}),
        // TextConfig({.text = "Option 3",
        // .color = whiteC,
        // .position = glm::vec2{0.f, 0.f},
        // .size = ddsize}),
        // });
        // auto dropdownConfig =
        // DropdownConfig({.textConfigs = ddtextConfigs,
        // .position = glm::vec2{3.f, 0.f},
        // .color = glm::vec4{0.4f, 0.3f, 0.8f, 1.f},
        // .size = glm::vec2{4.f, 1.f}});
        //
        // DropdownState dropdownState = DropdownState(0, false);
        // dropdown(get(), uuid({0, item++, 0}), dropdownConfig, dropdownState);

        // uuid active = get()->activeID;
        // uuid hot = get()->hotID;
        // log_info("active {} {} {} ", active.owner, active.item,
        // active.index); log_info("hot {} {} {} ", hot.owner, hot.item,
        // hot.index);
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
