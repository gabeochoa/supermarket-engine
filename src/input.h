#pragma once

#include "event.h"
#include "keycodes.h"
#include "pch.hpp"

struct Input {
    static bool IsKeyPressed(KeyCode key);
    static bool IsMouseButtonPressed(Mouse::MouseCode button);
    static glm::vec2 GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();
};

