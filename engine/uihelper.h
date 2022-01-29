
#pragma once

#include "camera.h"
#include "input.h"
#include "pch.hpp"
#include "renderer.h"

namespace IUI {

inline bool isKeyPressed(int keycode) {
    return Input::isKeyPressed(static_cast<Key::KeyCode>(keycode));
}

inline bool isMouseInside(
    std::shared_ptr<OrthoCameraController> cameraController, glm::vec4 rect) {
    auto mouseScreen = glm::vec3{Input::getMousePosition(), 0.f};
    mouseScreen.y = cameraController->camera.viewport.w - mouseScreen.y;

    auto mouse = screenToWorld(mouseScreen,                          //
                               cameraController->camera.view,        //
                               cameraController->camera.projection,  //
                               cameraController->camera.viewport     //
    );
    // log_warn("{} => {}, inside? {}", Input::getMousePosition(), mouse,
    // rect);
    return mouse.x >= rect.x && mouse.x <= rect.x + rect.z &&
           mouse.y >= rect.y && mouse.y <= rect.y + rect.w;
}

inline void drawForUI(glm::vec2 position, glm::vec2 size, float rotation,
                      glm::vec4 color, std::string texturename) {
    if (rotation > 5.f) {
        Renderer::drawQuadRotated(position,                //
                                  size,                    //
                                  glm::radians(rotation),  //
                                  color,                   //
                                  texturename);
        return;
    }
    Renderer::drawQuad(position, size, color, texturename);
}
}  // namespace IUI
