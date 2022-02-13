
#include "openglwindow.h"

#include "event.h"
#include "pch.hpp"

using namespace Mouse;

inline Key::KeyCode sfmlKeyToGLFWKey(sf::Keyboard::Key sfkey) {
    int sfkeycode = (int)sfkey;

    if (sfkeycode >= (int)sf::Keyboard::A &&
        sfkeycode <= (int)sf::Keyboard::Z) {
        return static_cast<Key::KeyCode>(65 + (int)sfkey);
    }

    if (sfkeycode >= (int)sf::Keyboard::Num0 &&
        sfkeycode <= (int)sf::Keyboard::Num9) {
        return static_cast<Key::KeyCode>(
            Key::KeyCode::D0 + ((int)sfkey - sf::Keyboard::Key::Num0));
    }

    if (sfkeycode >= (int)sf::Keyboard::Numpad0 &&
        sfkeycode <= (int)sf::Keyboard::Numpad9) {
        return static_cast<Key::KeyCode>(
            Key::KeyCode::KP0 + ((int)sfkey - sf::Keyboard::Key::Numpad0));
    }

    if (sfkeycode >= (int)sf::Keyboard::F1 &&
        sfkeycode <= (int)sf::Keyboard::F15) {
        return static_cast<Key::KeyCode>(Key::KeyCode::F1 +
                                         ((int)sfkey - sf::Keyboard::Key::F1));
    }

    if (sfkeycode == sf::Keyboard::Unknown) return Key::KeyCode();
    if (sfkeycode == sf::Keyboard::Escape) return Key::KeyCode::Escape;

    if (sfkeycode == sf::Keyboard::LControl) return Key::KeyCode::LeftControl;
    if (sfkeycode == sf::Keyboard::LShift) return Key::KeyCode::LeftShift;
    if (sfkeycode == sf::Keyboard::LAlt) return Key::KeyCode::LeftAlt;
    if (sfkeycode == sf::Keyboard::LSystem) return Key::KeyCode::LeftSuper;
    if (sfkeycode == sf::Keyboard::RControl) return Key::KeyCode::RightControl;
    if (sfkeycode == sf::Keyboard::RShift) return Key::KeyCode::RightShift;
    if (sfkeycode == sf::Keyboard::RAlt) return Key::KeyCode::RightAlt;
    if (sfkeycode == sf::Keyboard::RSystem) return Key::KeyCode::RightSuper;
    if (sfkeycode == sf::Keyboard::Menu) return Key::KeyCode::Menu;
    if (sfkeycode == sf::Keyboard::LBracket) return Key::KeyCode::LeftBracket;
    if (sfkeycode == sf::Keyboard::RBracket) return Key::KeyCode::RightBracket;
    if (sfkeycode == sf::Keyboard::Semicolon) return Key::KeyCode::Semicolon;
    if (sfkeycode == sf::Keyboard::Comma) return Key::KeyCode::Comma;
    if (sfkeycode == sf::Keyboard::Period) return Key::KeyCode::Period;
    if (sfkeycode == sf::Keyboard::Slash) return Key::KeyCode::Slash;
    if (sfkeycode == sf::Keyboard::Backslash) return Key::KeyCode::Backslash;
    if (sfkeycode == sf::Keyboard::Tilde) return Key::KeyCode::GraveAccent;
    if (sfkeycode == sf::Keyboard::Equal) return Key::KeyCode::Equal;
    if (sfkeycode == sf::Keyboard::Space) return Key::KeyCode::Space;
    if (sfkeycode == sf::Keyboard::Enter) return Key::KeyCode::Enter;
    if (sfkeycode == sf::Keyboard::Backspace) return Key::KeyCode::Backspace;
    if (sfkeycode == sf::Keyboard::Tab) return Key::KeyCode::Tab;
    if (sfkeycode == sf::Keyboard::PageUp) return Key::KeyCode::PageUp;
    if (sfkeycode == sf::Keyboard::PageDown) return Key::KeyCode::PageDown;
    if (sfkeycode == sf::Keyboard::End) return Key::KeyCode::End;
    if (sfkeycode == sf::Keyboard::Home) return Key::KeyCode::Home;
    if (sfkeycode == sf::Keyboard::Insert) return Key::KeyCode::Insert;
    if (sfkeycode == sf::Keyboard::Delete) return Key::KeyCode::Delete;
    if (sfkeycode == sf::Keyboard::Add) return Key::KeyCode::KPAdd;
    if (sfkeycode == sf::Keyboard::Subtract) return Key::KeyCode::KPSubtract;
    if (sfkeycode == sf::Keyboard::Multiply) return Key::KeyCode::KPMultiply;
    if (sfkeycode == sf::Keyboard::Divide) return Key::KeyCode::Slash;
    if (sfkeycode == sf::Keyboard::Left) return Key::KeyCode::Left;
    if (sfkeycode == sf::Keyboard::Right) return Key::KeyCode::Right;
    if (sfkeycode == sf::Keyboard::Up) return Key::KeyCode::Up;
    if (sfkeycode == sf::Keyboard::Down) return Key::KeyCode::Down;
    if (sfkeycode == sf::Keyboard::Pause) return Key::KeyCode::Pause;

    if (sfkeycode == sf::Keyboard::Quote) return Key::KeyCode::Apostrophe;
    if (sfkeycode == sf::Keyboard::Hyphen) return Key::KeyCode::Minus;
    if (sfkeycode == sf::Keyboard::KeyCount) return Key::KeyCode();
    return Key::KeyCode();
}

void OpenGLWindow::pollForEvents() {
    sf::Event evt;

    Event* event = nullptr;

    if (window->pollEvent(evt)) {
        switch (evt.type) {
            case sf::Event::Closed:
                event = new WindowCloseEvent();
                break;
            case sf::Event::Resized:
                event = new WindowResizeEvent(evt.size.width, evt.size.height);
                break;
            case sf::Event::TextEntered:
                // TODO figure out if theres a way to GLFW_REPEAT
                event = new CharPressedEvent((int)evt.text.unicode, 0);
                break;
            case sf::Event::KeyPressed:
                // TODO figure out if these keys are the same as glfw's
                // TODO figure out if theres a way to GLFW_REPEAT
                event =
                    new KeyPressedEvent((int)sfmlKeyToGLFWKey(evt.key.code), 0);
                break;
            case sf::Event::KeyReleased:
                event =
                    new KeyReleasedEvent((int)sfmlKeyToGLFWKey(evt.key.code));
                break;
            case sf::Event::MouseButtonPressed:
                event = new MouseButtonPressedEvent(evt.mouseButton.button);
                break;
            case sf::Event::MouseButtonReleased:
                event = new MouseButtonReleasedEvent(evt.mouseButton.button);
                break;
            case sf::Event::MouseMoved:
                event =
                    new MouseMovedEvent(evt.mouseButton.x, evt.mouseButton.y);
                break;
            case sf::Event::MouseWheelScrolled:
                // TODO do we need to support horizontal scrolling ?
                // right now we just treat both as vertical
                event = new MouseScrolledEvent(evt.mouseWheelScroll.x,
                                               evt.mouseWheelScroll.y);
                break;
            case sf::Event::MouseEntered:
            case sf::Event::MouseLeft:
            case sf::Event::JoystickButtonPressed:
            case sf::Event::JoystickButtonReleased:
            case sf::Event::JoystickMoved:
            case sf::Event::JoystickConnected:
            case sf::Event::JoystickDisconnected:
            case sf::Event::TouchBegan:
            case sf::Event::TouchMoved:
            case sf::Event::TouchEnded:
            case sf::Event::SensorChanged:
            case sf::Event::Count:
            case sf::Event::MouseWheelMoved:
            case sf::Event::LostFocus:
            case sf::Event::GainedFocus:
                // TODO lots of events we arent handling yet
                break;
        }
        if (event != nullptr) {
            this->info.callback(*event);
            delete event;
        }
    }
}

void OpenGLWindow::update() {
    pollForEvents();
    window->display();
}

void OpenGLWindow::init(const WindowConfig& config) {
    info.title = config.title;
    info.width = config.width;
    info.height = config.height;

    log_trace("Creating a new window {} ({}, {})", info.title, info.width,
              info.height);

    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 2;
    settings.depthBits = 32;
    settings.stencilBits = 0;
    settings.attributeFlags = sf::ContextSettings::Attribute::Core;

    sf::RenderWindow* win =
        new sf::RenderWindow(sf::VideoMode(info.width, info.height), info.title,
                             sf::Style::Default, settings);
    M_ASSERT(win, "Failed to create window");
    win->resetGLStates();

    // store our new window as a Window*
    window = win;

    window->setActive(true);
    setVSync(true);
}

Window* Window::create(const WindowConfig& config) {
    return new OpenGLWindow(config);
}
