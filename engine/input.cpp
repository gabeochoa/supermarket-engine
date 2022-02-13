
#include "pch.hpp"

#include "input.h" 
#include "event.h" 
#include "app.h" 

bool Input::isKeyPressedNoRepeat(const Key::KeyCode key) {
    return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key));
}

bool Input::isKeyPressed(const Key::KeyCode key) {
    return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(key));
    // TODO support repeat? 
    // return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::isKeyReleased(const Key::KeyCode key) {
    return !Input::isKeyPressed(key);
}

bool Input::isMouseButtonPressed(const Mouse::MouseCode button) {
    return sf::Mouse::isButtonPressed(static_cast<sf::Mouse::Button>(button));
}

glm::vec2 Input::getMousePosition() {
    sf::Vector2i pos = sf::Mouse::getPosition();
    return {(float)pos.x, (float)pos.y};
}

glm::vec2 Input::getMousePositionInWindow(sf::Window window) {
    sf::Vector2i pos = sf::Mouse::getPosition(window);
    return {(float)pos.x, (float)pos.y};
}

float Input::getMouseX() { return getMousePosition().x; }

float Input::getMouseY() { return getMousePosition().y; }
