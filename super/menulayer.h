
#pragma once

#include "../engine/camera.h"
#include "../engine/layer.h"
#include "../engine/pch.hpp"
#include "../engine/ui.h"
//
#include "entities.h"

struct CameraPositionInterpolation {
    int camPosIndex = 0;
    std::array<glm::vec2, 3> camPositions = {{
        glm::vec2{0.f, 0.f},
        glm::vec2{50.f, 0.f},
        glm::vec2{150.f, 0.f},
    }};
    glm::vec2 lastCamPos;
    glm::vec2 targetCamPos;
    LinearInterp xinterp;
    LinearInterp yinterp;
    int steps = 100;

    CameraPositionInterpolation() {
        lastCamPos = camPositions[0];
        targetCamPos = camPositions[0];
        xinterp = LinearInterp(lastCamPos.x, targetCamPos.x, steps);
        yinterp = LinearInterp(lastCamPos.y, targetCamPos.y, steps);
    }

    void update_target_camera_position() {
        targetCamPos = camPositions[camPosIndex];
        xinterp = LinearInterp(lastCamPos.x, targetCamPos.x, steps);
        yinterp = LinearInterp(lastCamPos.y, targetCamPos.y, steps);
    }

    void set(int index) {
        lastCamPos = camPositions[camPosIndex];
        camPosIndex = index;
        update_target_camera_position();
    }

    void back() {
        lastCamPos = camPositions[camPosIndex];
        camPosIndex = fmax(0.f, camPosIndex - 1);
        update_target_camera_position();
    }

    void next() {
        lastCamPos = camPositions[camPosIndex];
        camPosIndex = fmin(camPosIndex + 1, camPositions.size());
        update_target_camera_position();
    }
};

// TODO add a default texture that isnt just white
// make it obvious that the texture you are trying
// to draw doesnt exist

struct MenuLayer : public Layer {
    float value = 0.08f;
    std::string content = "";
    const int TYPABLE_START = 32;
    const int TYPABLE_END = 126;
    CameraPositionInterpolation camPosInterp;
    glm::vec2 b1Pos;
    glm::vec2 b1Size;
    float b1extra = 0.f;
    std::shared_ptr<Billboard> left_convey;
    std::shared_ptr<Billboard> right_convey;

    std::vector<std::shared_ptr<Billboard>> screen0;
    std::vector<std::shared_ptr<Billboard>> billys;

    MenuLayer() : Layer("Supermarket") {
        menuCameraController.reset(
            new OrthoCameraController(WIN_RATIO, 10.f, 0.f, 0.f));

        // TODO have to make sure this resource
        // exists as part of the engine
        // and not the game textures
        Renderer::addTexture("./resources/menu_bg.png");
        Renderer::addSubtexture("menu_bg", "menu_1_bg", 0, 0, 53.f, 32.f);
        Renderer::addSubtexture("menu_bg", "menu_2_bg", 1, 0, 56.f, 32.f);

        Renderer::addTexture("./resources/transparent.png");
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

        b1Pos = camPosInterp.camPositions[0] + glm::vec2{7.f, 5.f};
        b1Size = glm::vec2{54.f, 32.f};
        std::shared_ptr<Billboard> b1;
        b1.reset(new Billboard(b1Pos,           //
                               b1Size,          //
                               0.f,             //
                               glm::vec4{1.f},  //
                               "menu_1_bg"));
        b1->center = false;
        screen0.push_back(b1);

        std::shared_ptr<Billboard> b2;
        b2.reset(new Billboard(b1Pos + glm::vec2{b1Size.x, 0.f},  //
                               b1Size,                            //
                               0.f,                               //
                               glm::vec4{1.f},                    //
                               "menu_1_bg"));
        b2->center = false;
        screen0.push_back(b2);

        std::shared_ptr<Billboard> bg_2;
        bg_2.reset(new Billboard(
            camPosInterp.camPositions[1] + glm::vec2{7.f, 5.f},  //
            glm::vec2{54.f, 32.f},                               //
            0.f,                                                 //
            glm::vec4{1.f},                                      //
            "menu_2_bg"));
        billys.push_back(bg_2);
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
            if (camPosInterp.camPosIndex == 0) {
                App::get().running = false;
            } else {
                camPosInterp.back();
            }
            return true;
        }

        if (camPosInterp.camPosIndex == 0) {
            camPosInterp.next();
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

        if (Menu::get().state != Menu::State::Root) {
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

        auto mvt_speed = 15.f;

        // Target position is pos0 but we have extra,
        // move the right one back to where it was
        // by how much extra we had to wait after we
        // went to pos1
        if (b1extra > 0.f && camPosInterp.camPosIndex == 0) {
            right_convey->position.x += b1extra;
            b1extra = 0.f;
        }

        // always have the left one be the left one
        left_convey = screen0[0];
        right_convey = screen0[1];
        if (screen0[0]->position.x > screen0[1]->position.x) {
            left_convey = screen0[1];
            right_convey = screen0[0];
        }

        // if we are at pos0, then scroll the conveyer
        if (glm::distance(
                glm::vec2{
                    menuCameraController->camera.position.x,
                    menuCameraController->camera.position.y,
                },
                camPosInterp.camPositions[0]) < 10.f) {
            left_convey->position.x -= mvt_speed * dt.s();
            right_convey->position.x -= mvt_speed * dt.s();

            if (left_convey->position.x + left_convey->size.x < 0.f) {
                left_convey->position.x = left_convey->size.x;
            }
        } else {
            if (right_convey->position.x + right_convey->size.x >
                camPosInterp.camPositions[1].x) {
                float dst = mvt_speed * dt.s();
                right_convey->position.x -= dst;
                b1extra += dst;
            }
        }
        left_convey->render();
        right_convey->render();

        for (auto& billy : billys) {
            billy->render();
        }
        Renderer::end();
    }

    void ui_test() {
        menuCameraController->camera.position =
            glm::vec3{camPosInterp.xinterp.next(), camPosInterp.yinterp.next(),
                      menuCameraController->camera.position.z};

        using namespace IUI;
        int item = 0;
        int parent = 0;
        begin(menuCameraController);
        {
            auto startPos = glm::vec2{-17.f, -10.f};

            // glm::vec2{menuCameraController->camera.position.x - 17.f,
            // menuCameraController->camera.position.y + -10.f};

            auto textConfig = IUI::WidgetConfig({
                .text = "Tap to continue",
                .position = glm::vec2{1.f, 3.f},           //
                .size = glm::vec2{2.f, 2.f},               //
                .color = glm::vec4{1.f, 1.0f, 1.0f, 1.f},  //
            });
            auto buttonConfig = IUI::WidgetConfig({
                .position = startPos,           //
                .size = glm::vec2{40.f, 20.f},  //
                .transparent = true,            //
                .child = &textConfig,           //
                .texture = "transparent",       //
            });

            if (IUI::button_with_label(IUI::uuid({parent, item++, 0}),
                                       buttonConfig)) {
                camPosInterp.set(1);
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
        // log_warn(event.toString().c_str());
        if (Menu::get().state != Menu::State::Root) return;

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
