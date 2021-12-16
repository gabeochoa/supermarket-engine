
#pragma once
#include "keycodes.h"
#include "pch.hpp"

enum class EventType {
    None = 0,
    WindowClose,
    WindowResize,
    WindowFocus,
    WindowLostFocus,
    WindowMoved,
    AppTick,
    AppUpdate,
    AppRender,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled,
    KeyPressed,
    KeyReleased,
    KeyTyped,
};

#define BIT(x) (1 << x)
enum EventCategory {
    EventCategoryNone = 0,
    EventCategoryApplication = BIT(0),
    EventCategoryInput = BIT(1),
    EventCategoryKeyboard = BIT(2),
    EventCategoryMouse = BIT(3),
    EventCategoryMouseButton = BIT(4),
};

#define MACRO_EVENT_TYPE(type)                                   \
    static EventType getStaticType() { return EventType::type; } \
    virtual EventType getEventType() const override {            \
        return getStaticType();                                  \
    }                                                            \
    virtual const char* getName() const override { return #type; }

#define MACRO_EVENT_CATEGORY(cat) \
    virtual int getCategoryFlags() const override { return cat; }

struct Event {
    bool handled = false;

    friend struct EventDispatcher;

    virtual ~Event() {}

    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;
    virtual std::string toString() const { return getName(); }
    bool isInCategory(EventCategory cat) const {
        return getCategoryFlags() & cat;
    }
};

inline std::ostream& operator<<(std::ostream& os, Event const& e) {
    return os << e.toString();
}

struct EventDispatcher {
    EventDispatcher(Event& e) : event(e) {}
    Event& event;

    template <typename T>
    bool dispatch(std::function<bool(T&)> func) {
        if (event.getEventType() == T::getStaticType()) {
            event.handled = func(*(T*)&event);
            return true;
        }
        return false;
    }
};

struct KeyEvent : public Event {
    KeyEvent(int k) : keycode(k) {}
    int keycode;
    int getKeyCode() const { return keycode; }

    MACRO_EVENT_CATEGORY(EventCategoryKeyboard | EventCategoryInput);
};

struct KeyPressedEvent : public KeyEvent {
    MACRO_EVENT_TYPE(KeyPressed)

    KeyPressedEvent(int k, int r) : KeyEvent(k), repeatCount(r) {}
    int repeatCount;
    int getRepeatCount() const { return repeatCount; }

    std::string toString() const override {
        return fmt::format("KeyPressedEvent: {} ({} repeats)", keycode,
                           repeatCount);
    }
};

struct KeyReleasedEvent : public KeyEvent {
    MACRO_EVENT_TYPE(KeyReleased)
    KeyReleasedEvent(int k) : KeyEvent(k) {}
    std::string toString() const override {
        return fmt::format("KeyReleasedEvent: {}", keycode);
    }
};

struct KeyTypedEvent : public KeyEvent {
    MACRO_EVENT_TYPE(KeyTyped)
    KeyTypedEvent(int k) : KeyEvent(k) {}
    std::string toString() const override {
        return fmt::format("KeyTypedEvent: {}", keycode);
    }
};

namespace Mouse {
enum MouseCode {
    // From glfw3.h
    Button0 = 0,
    Button1 = 1,
    Button2 = 2,
    Button3 = 3,
    Button4 = 4,
    Button5 = 5,
    Button6 = 6,
    Button7 = 7,

    ButtonLast = Button7,
    ButtonLeft = Button0,
    ButtonRight = Button1,
    ButtonMiddle = Button2
};

class MouseMovedEvent : public Event {
   public:
    MouseMovedEvent(const float x, const float y) : mouseX(x), mouseY(y) {}

    float x() const { return mouseX; }
    float y() const { return mouseY; }

    std::string toString() const override {
        return fmt::format("MouseMovedEvent: {},{}", mouseX, mouseY);
    }

    MACRO_EVENT_TYPE(MouseMoved)
    MACRO_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput)
   private:
    float mouseX, mouseY;
};

class MouseScrolledEvent : public Event {
   public:
    MouseScrolledEvent(const float xOffset, const float yOffset)
        : XOffset(xOffset), YOffset(yOffset) {}

    float GetXOffset() const { return XOffset; }
    float GetYOffset() const { return YOffset; }

    std::string toString() const override {
        return fmt::format("MouseScrolledEvent: {},{}", GetXOffset(),
                           GetYOffset());
    }

    MACRO_EVENT_TYPE(MouseScrolled)
    MACRO_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput)
   private:
    float XOffset, YOffset;
};

struct MouseButtonEvent : public Event {
   public:
    MouseCode GetMouseButton() const { return button; }

    MACRO_EVENT_CATEGORY(EventCategoryMouse | EventCategoryInput |
                         EventCategoryMouseButton)
   protected:
    MouseButtonEvent(const MouseCode b) : button(b) {}

    MouseCode button;
};

struct MouseButtonPressedEvent : public MouseButtonEvent {
   public:
    MouseButtonPressedEvent(const int b)
        : MouseButtonEvent(static_cast<MouseCode>(b)) {}
    MouseButtonPressedEvent(const MouseCode b) : MouseButtonEvent(b) {}

    std::string toString() const override {
        return fmt::format("MouseButtonPressedEvent: {}", button);
    }

    MACRO_EVENT_TYPE(MouseButtonPressed)
};

struct MouseButtonReleasedEvent : public MouseButtonEvent {
   public:
    MouseButtonReleasedEvent(const int b)
        : MouseButtonEvent(static_cast<MouseCode>(b)) {}
    MouseButtonReleasedEvent(const MouseCode b) : MouseButtonEvent(b) {}

    std::string toString() const override {
        return fmt::format("MouseButtonReleasedEvent: ", button);
    }

    MACRO_EVENT_TYPE(MouseButtonReleased)
};
}  // namespace Mouse

class WindowResizeEvent : public Event {
   public:
    WindowResizeEvent(unsigned int width, unsigned int height)
        : Width(width), Height(height) {}

    unsigned int width() const { return Width; }
    unsigned int height() const { return Height; }

    std::string toString() const override {
        return fmt::format("WindowResizeEvent: {}, {}", Width, Height);
    }

    MACRO_EVENT_TYPE(WindowResize)
    MACRO_EVENT_CATEGORY(EventCategoryApplication)
   private:
    unsigned int Width, Height;
};

class WindowCloseEvent : public Event {
   public:
    WindowCloseEvent() = default;

    MACRO_EVENT_TYPE(WindowClose)
    MACRO_EVENT_CATEGORY(EventCategoryApplication)
};

class AppTickEvent : public Event {
   public:
    AppTickEvent() = default;

    MACRO_EVENT_TYPE(AppTick)
    MACRO_EVENT_CATEGORY(EventCategoryApplication)
};

class AppUpdateEvent : public Event {
   public:
    AppUpdateEvent() = default;

    MACRO_EVENT_TYPE(AppUpdate)
    MACRO_EVENT_CATEGORY(EventCategoryApplication)
};

class AppRenderEvent : public Event {
   public:
    AppRenderEvent() = default;

    MACRO_EVENT_TYPE(AppRender)
    MACRO_EVENT_CATEGORY(EventCategoryApplication)
};
