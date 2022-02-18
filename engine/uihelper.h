
#pragma once

#include "camera.h"
#include "font.h"
#include "input.h"
#include "pch.hpp"
#include "renderer.h"
#include "ui.h"

namespace GOUI {

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

inline void init_uicontext(
    UIContext* uicontext,
    std::shared_ptr<OrthoCameraController> cameraController) {
    auto genFontTexture = [](std::string fontname, std::wstring phrase,
                             bool is_temporary) {
        auto texture =
            fetch_texture_for_phrase(phrase, fontname.c_str(), is_temporary);
        if (!texture) return UIContext::FontPhraseTexInfo{};
        return UIContext::FontPhraseTexInfo{
            .textureName = texture->name,
            .width = texture->width,
            .height = texture->height,
            .fontSize = FONT_SIZE,
        };
    };

    uicontext->init(std::bind(&GOUI::isMouseInside, cameraController,
                              std::placeholders::_1),
                    &GOUI::isKeyPressed, &GOUI::drawForUI, genFontTexture);

    uicontext->init_keys(UIContext::KeyCodes{
        .widgetNext = Key::getMapping("Widget Next"),              //
        .widgetMod = Key::getMapping("Widget Mod"),                //
        .widgetPress = Key::getMapping("Widget Press"),            //
        .valueUp = Key::getMapping("Value Up"),                    //
        .valueDown = Key::getMapping("Value Down"),                //
        .textBackspace = Key::getMapping("Text Backspace"),        //
        .commandEnter = Key::getMapping("Command Enter"),          //
        .clearCommandLine = Key::getMapping("Clear Command Line")  //
    });
}

}  // namespace GOUI
