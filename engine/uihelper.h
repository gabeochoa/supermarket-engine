
#pragma once

#include "camera.h"
#include "pch.hpp"

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
