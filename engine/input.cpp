
#include "pch.hpp"

#include "input.h" 
#include "event.h" 
#include "app.h" 

bool Input::isKeyPressedNoRepeat(const Key::KeyCode key) {
    auto* window =
        static_cast<GLFWwindow*>(App::get().getWindow().getNativeWindow());
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_PRESS;
}

bool Input::isKeyPressed(const Key::KeyCode key) {
    auto* window =
        static_cast<GLFWwindow*>(App::get().getWindow().getNativeWindow());
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isKeyReleased(const Key::KeyCode key) {
    auto* window =
        static_cast<GLFWwindow*>(App::get().getWindow().getNativeWindow());
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_RELEASE;
}

bool Input::isMouseButtonPressed(const Mouse::MouseCode button) {
    auto* window =
        static_cast<GLFWwindow*>(App::get().getWindow().getNativeWindow());
    auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}

glm::vec2 Input::getMousePosition() {
    auto* window =
        static_cast<GLFWwindow*>(App::get().getWindow().getNativeWindow());
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    return {(float)xpos, (float)ypos};
}

float Input::getMouseX() { return getMousePosition().x; }

float Input::getMouseY() { return getMousePosition().y; }
