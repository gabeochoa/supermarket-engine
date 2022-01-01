
#pragma once

#include "../engine/camera.h"
#include "../engine/layer.h"
#include "../engine/pch.hpp"
#include "../engine/time.h"
#include "../engine/ui.h"
#include "global.h"

struct TerminalLayer : public Layer {
    const glm::vec2 camTopLeft = {35.f, 19.5f};
    const glm::vec2 camBottomRight = {35.f, -18.f};
    glm::vec4 rect = glm::vec4{200.f, 1000.f, 1500.f, 200.f};
    // TODO ive just realized that UUID 000 is basically every layer
    // probably instead of using parent we should use layer as the first id...
    IUI::uuid drawer_uuid = IUI::uuid({id, 0, 0});

    TerminalLayer() : Layer("Debug Terminal") {
        isMinimized = true;

        terminalCameraController.reset(new OrthoCameraController(WIN_RATIO));
        terminalCameraController->setZoomLevel(20.f);
        terminalCameraController->camera.setViewport({0, 0, WIN_W, WIN_H});
        terminalCameraController->movementEnabled = false;
        terminalCameraController->rotationEnabled = false;
        terminalCameraController->zoomEnabled = false;
        terminalCameraController->resizeEnabled = false;
    }

    virtual ~TerminalLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    glm::vec2 convertUIPos(glm::vec2 pos, bool flipy = true) {
        auto y = flipy ? WIN_H - pos.y : pos.y;
        return screenToWorld(glm::vec3{pos.x, y, 0.f},
                             terminalCameraController->camera.view,
                             terminalCameraController->camera.projection,
                             terminalCameraController->camera.viewport);
    }

    std::array<glm::vec2, 2> getPositionSizeForUIRect(glm::vec4 rect) {
        glm::vec2 position = convertUIPos(glm::vec2{rect.x, rect.y});
        glm::vec2 size = convertUIPos(glm::vec2{rect.z, rect.w});
        return std::array{
            position + (size * 0.5f),
            size,
        };
    }

    virtual void onUpdate(Time dt) override {
        if (isMinimized) {
            return;
        }

        log_trace("{:.2}s ({:.2} ms) ", dt.s(), dt.ms());
        prof give_me_a_name(__PROFILE_FUNC__);  //
        terminalCameraController->onUpdate(dt);
        terminalCameraController->camera.setProjection(0.f, WIN_W, WIN_H, 0.f);

        Renderer::begin(terminalCameraController->camera);

        using namespace IUI;
        UIFrame BandE(terminalCameraController);
        int item = 0;

        std::vector<std::function<bool(uuid)>> children;

        float h1_fs = 64.f;
        float p_fs = 32.f;

        children.push_back([&](uuid id) {
            auto textConfig = WidgetConfig({
                .color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
                .position = convertUIPos({0, h1_fs + 1.f}),
                .size = glm::vec2{h1_fs, -h1_fs},
                .text = "Terminal",
            });
            return text(id, textConfig);
        });

        auto drawer_location = getPositionSizeForUIRect({0, 0, WIN_W, 400});

        children.push_back([&](uuid id) {
            auto cfsize = glm::vec2{drawer_location[1].x, 120.f};
            auto commandFieldConfig = WidgetConfig({
                .position = glm::vec2{drawer_location[0].x,
                                      drawer_location[0].y +
                                          drawer_location[1].y - cfsize.y},
                .size = cfsize,
            });
            return commandfield(id, commandFieldConfig);
        });

        drawer(drawer_uuid,
               WidgetConfig({
                   .color = blue,
                   .position = drawer_location[0],
                   .size = drawer_location[1],
               }),
               children);

        Renderer::end();
    }

    bool onKeyPressed(KeyPressedEvent event) {
        if (event.keycode == Key::mapping["Toggle Debugger"]) {
            isMinimized = !isMinimized;
            // TODO this requires us to know the internal setup of state
            // probably just have drawer take the pct open
            auto state =
                IUI::get()->statemanager.get_as<IUI::DrawerState>(drawer_uuid);
            if (state) {
                state->heightPct = 0.f;
            }
        }
        // TODO is there a way for us to not have to do this?
        // or make it required so you cant even start without it accidentally
        if (IUI::get()->processKeyPressEvent(event)) {
            return true;
        }
        // TODO since we have no way to have IUI return true (yet!)
        // eat all keypresses while we are open
        if (!isMinimized) return true;
        return false;
    }

    virtual void onEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<KeyPressedEvent>(std::bind(
            &TerminalLayer::onKeyPressed, this, std::placeholders::_1));
        terminalCameraController->onEvent(event);
        dispatcher.dispatch<CharPressedEvent>(
            IUI::get()->getCharPressHandler());
    }
};

