#pragma once

#include "event.h"
#include "keycodes.h"
#include "pch.hpp"

struct Input {
    static bool isKeyPressedNoRepeat(Key::KeyCode key);
    static bool isKeyPressed(Key::KeyCode key);
    static bool isKeyReleased(Key::KeyCode key);
    static bool isMouseButtonPressed(Mouse::MouseCode button);
    static glm::vec2 getMousePosition();
    static glm::vec2 getMousePositionInWindow(sf::Window window);
    static float getMouseX();
    static float getMouseY();
};

