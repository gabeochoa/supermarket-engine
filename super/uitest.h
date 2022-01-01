
#pragma once

#include "../engine/app.h"
#include "../engine/camera.h"
#include "../engine/edit.h"
#include "../engine/file.h"
#include "../engine/font.h"
#include "../engine/layer.h"
#include "../engine/pch.hpp"
#include "../engine/ui.h"
//
#include "global.h"
//
#include "entities.h"
#include "menu.h"

struct UITestLayer : public Layer {
    float value = 0.08f;
    std::wstring content = L"";
    const int TYPABLE_START = 32;
    const int TYPABLE_END = 126;
    bool checkbox_state = false;
    int dropdownIndex = 0;
    bool dropdownState = false;
    float upperCaseRotation = 0.f;
    bool camHasMovement = false;

    std::shared_ptr<Billboard> billy;

    UITestLayer() : Layer("UI Test") {
        Menu::get().state = Menu::State::UITest;

        uiTestCameraController.reset(
            new OrthoCameraController(WIN_RATIO, 10.f, 5.f, 0.f));
        uiTestCameraController->camera.setPosition(glm::vec3{15.f, 0.f, 0.f});
        camHasMovement = uiTestCameraController->movementEnabled;

        uiTestCameraController->camera.setViewport(
            glm::vec4{0, 0, WIN_W, WIN_H});

        IUI::init_context();
        GLOBALS.set<float>("slider_val", &value);
    }

    virtual ~UITestLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    bool onKeyPressed(KeyPressedEvent& event) {
        if (event.keycode == Key::mapping["Esc"]) {
            App::get().running = false;
            return true;
        }
        if (IUI::get()->processKeyPressEvent(event)) {
            return true;
        }
        return false;
    }

    virtual void onUpdate(Time dt) override {
        log_trace("{:.2}s ({:.2} ms) ", dt.s(), dt.ms());
        prof(__PROFILE_FUNC__);

        if (Menu::get().state != Menu::State::UITest) {
            return;
        }

        uiTestCameraController->onUpdate(dt);

        gltInit();
        gltBeginDraw();
        gltColor(1.0f, 1.0f, 1.0f, 1.0f);
        GLTtext* text = gltCreateText();
        gltSetText(text, stateToString(Menu::get().state));
        gltDrawText2D(text, 150, 150, 5.f);
        gltEndDraw();
        gltDeleteText(text);
        gltTerminate();

        Renderer::begin(uiTestCameraController->camera);
        ui_test(dt);
        Renderer::end();
    }

    void ui_test(Time dt) {
        using namespace IUI;
        int item = 0;
        {
            // uses RAII to handle begin()/end() automatically
            UIFrame BandE(uiTestCameraController);

            if (button(uuid({id, item++, 0}),
                       WidgetConfig({.position = glm::vec2{0.f, 0.f},
                                     .size = glm::vec2{2.f, 1.f}}))) {
                log_info("clicked button");
                Menu::get().state = Menu::State::Root;
            }
            if (button(uuid({id, item++, 0}),
                       WidgetConfig({.position = glm::vec2{3.f, -2.f},
                                     .size = glm::vec2{6.f, 1.f},
                                     .text = "open file dialog"}))) {
                auto files = getFilesFromUser();
                for (auto file : files)
                    log_info("You chose the file: {}", file);
            }
            if (button(uuid({id, item++, 0}),
                       WidgetConfig({.position = glm::vec2{2.5f, 0.f},
                                     .size = glm::vec2{6.f, 1.f},
                                     .text = "open mesage box"}))) {
                auto result =
                    openMessage("Example", "This is an example message box");
                auto str =
                    (result == 0 ? "cancel" : (result == 1 ? "yes" : "no"));
                log_info("they clicked: {}", str);
            }

            if (slider(uuid({id, item++, 0}),
                       WidgetConfig({
                           .position = glm::vec2{9.f, 0.f},
                           .size = glm::vec2{1.f, 3.f},
                           .vertical = true,
                       }),
                       GLOBALS.get_ptr<float>("slider_val"), 0.08f, 0.95f)) {
                // log_info("idk moved slider? ");
            }

            if (slider(uuid({id, item++, 0}),
                       WidgetConfig({
                           .position = glm::vec2{11.f, 3.f},
                           .size = glm::vec2{3.f, 1.f},
                           .vertical = false,
                       }),
                       GLOBALS.get_ptr<float>("slider_val"), 0.08f, 0.95f)) {
                // log_info("idk moved slider? ");
            }

            upperCaseRotation += 90.f * dt.s();
            auto upperCaseConfig =
                WidgetConfig({.color = glm::vec4{1.0, 0.8f, 0.5f, 1.0f},
                              .position = glm::vec2{0.f, -6.f},
                              .rotation = upperCaseRotation,
                              .size = glm::vec2{1.f, 1.f},
                              .text = "THE FIVE BOXING WIZARDS JUMP QUICKLY"});

            auto lowerCaseConfig =
                WidgetConfig({.color = glm::vec4{0.8, 0.3f, 0.7f, 1.0f},
                              .position = glm::vec2{0.f, -8.f},
                              .size = glm::vec2{1.f, 1.f},
                              .text = "the five boxing wizards jump quickly"});

            auto numbersConfig =
                WidgetConfig({.color = glm::vec4{0.7, 0.5f, 0.8f, 1.0f},
                              .position = glm::vec2{0.f, -10.f},
                              .size = glm::vec2{1.f, 1.f},
                              .text = "0123456789"});

            auto extrasConfig =
                WidgetConfig({.color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
                              .position = glm::vec2{0.f, -12.f},
                              .size = glm::vec2{1.f, 1.f},
                              .text = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"});

            auto hiraganaConfig = WidgetConfig({
                .color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
                .font = "Sazanami-Hanazono-Mincho",
                .position = glm::vec2{0.f, -14.f},
                .size = glm::vec2{1.f, 1.f},
                .text = "Hiragana: "
                        "\xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3"
                        "\x81\x91\xe3\x81\x93",
            });
            auto kanjiConfig = WidgetConfig({
                .color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
                .font = "Sazanami-Hanazono-Mincho",
                .position = glm::vec2{0.f, -16.f},
                .size = glm::vec2{1.f, 1.f},
                .text = "Kanji: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e",
            });

            text(uuid({id, item++, 0}), upperCaseConfig);
            text(uuid({id, item++, 0}), lowerCaseConfig);
            text(uuid({id, item++, 0}), numbersConfig);
            text(uuid({id, item++, 0}), extrasConfig);
            text(uuid({id, item++, 0}), hiraganaConfig);
            text(uuid({id, item++, 0}), kanjiConfig);

            {
                uuid textFieldID = uuid({id, item++, 0});
                if (textfield(textFieldID,
                              WidgetConfig({.position = glm::vec2{2.f, 2.f},
                                            .size = glm::vec2{6.f, 1.f}}),
                              content)) {
                    log_info("{}", to_string(content));
                }

                // In this case we want to lock the camera when typing in
                // this specific textfield
                // TODO should this be the default?
                // TODO should this live in the textFieldConfig?
                uiTestCameraController->movementEnabled = camHasMovement;
                if (uiTestCameraController->movementEnabled &&
                    IUI::has_kb_focus(textFieldID)) {
                    uiTestCameraController->movementEnabled = false;
                }
            }

            {
                uuid textFieldID = uuid({id, item++, 0});
                if (commandfield(textFieldID,
                                 WidgetConfig({.position = glm::vec2{2.f, 4.f},
                                               .size = glm::vec2{6.f, 1.f}}))) {
                    log_info("{}", EDITOR_COMMANDS.command_history.back());
                }

                // See now im doing this again, so yea probably I should solve
                // this separately from here, we have the camera in Frame so
                // maybe we should just take the controller and take control
                uiTestCameraController->movementEnabled = camHasMovement;
                if (uiTestCameraController->movementEnabled &&
                    IUI::has_kb_focus(textFieldID)) {
                    uiTestCameraController->movementEnabled = false;
                }
            }

            auto tapToContiueText = WidgetConfig({
                .position = glm::vec2{0.5f, 0.f},
                .size = glm::vec2{0.75f, 0.75f},
                .text = "Tap to continue",
            });

            if (IUI::button_with_label(
                    IUI::uuid({id, item++, 0}),
                    IUI::WidgetConfig({
                        .child = &tapToContiueText,                 //
                        .color = glm::vec4{0.3f, 0.9f, 0.5f, 1.f},  //
                        .position = glm::vec2{12.f, 1.f},           //
                        .size = glm::vec2{8.f, 1.f},                //
                        .transparent = false,                       //
                    })                                              //
                    )) {
                Menu::get().state = Menu::State::UITest;
            }

            if (IUI::checkbox(                   //
                    IUI::uuid({id, item++, 0}),  //
                    IUI::WidgetConfig({
                        .color = glm::vec4{0.6f, 0.3f, 0.3f, 1.f},  //
                        .position = glm::vec2{16.f, 3.f},           //
                        .size = glm::vec2{1.f, 1.f},                //
                        .transparent = false,                       //
                    }),                                             //
                    &checkbox_state)) {
                log_info("checkbox changed {}", checkbox_state);
            }

            {
                std::vector<WidgetConfig> dropdownConfigs;
                dropdownConfigs.push_back(
                    IUI::WidgetConfig({.text = "option A"}));
                dropdownConfigs.push_back(
                    IUI::WidgetConfig({.text = "option B"}));
                dropdownConfigs.push_back(
                    IUI::WidgetConfig({.text = "long option"}));
                dropdownConfigs.push_back(
                    IUI::WidgetConfig({.text = "really really long option"}));

                WidgetConfig dropdownMain = IUI::WidgetConfig({
                    .color = glm::vec4{0.3f, 0.9f, 0.5f, 1.f},  //
                    .position = glm::vec2{12.f, -1.f},          //
                    .size = glm::vec2{8.5f, 1.f},               //
                    .text = "",                                 //
                    .transparent = false,                       //
                });

                if (IUI::dropdown(IUI::uuid({id, item++, 0}), dropdownMain,
                                  dropdownConfigs, &dropdownState,
                                  &dropdownIndex)) {
                    dropdownState = !dropdownState;
                }
            }

            // end with BandE
        }
    }

    bool onMouseButtonPressed(Mouse::MouseButtonPressedEvent& e) {
        (void)e;
        if (e.GetMouseButton() == Mouse::MouseCode::ButtonRight) {
            Menu::get().state = Menu::State::Game;
        }
        return false;
    }

    virtual void onEvent(Event& event) override {
        // log_warn(event.toString().c_str());
        if (Menu::get().state != Menu::State::UITest) return;

        uiTestCameraController->onEvent(event);
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(std::bind(
            &UITestLayer::onMouseButtonPressed, this, std::placeholders::_1));
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&UITestLayer::onKeyPressed, this, std::placeholders::_1));
        dispatcher.dispatch<CharPressedEvent>(
            IUI::get()->getCharPressHandler());
    }
};
