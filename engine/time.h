
#pragma once

#include "pch.hpp"

struct Time {
    float last;
    float delta;

    void start() { last = glfwGetTime(); }

    void end() {
        float now = (float)glfwGetTime();
        delta = now - last;
        last = now;
    }

    operator float() { return s(); }
    float ms() const { return delta * 1000.f; }
    float s() const { return delta; }
};
