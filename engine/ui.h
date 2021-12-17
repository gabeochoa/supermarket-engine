
#pragma once

#include "input.h"
#include "pch.hpp"

namespace IUI {

struct uuid {
    int owner;
    int item;
    int index;

    bool operator==(const uuid& other) const {
        return owner == other.owner && item == other.item &&
               index == other.index;
    }

    bool operator<(const uuid& other) const {
        return owner < other.owner && item < other.item && index < other.index;
    }
};

struct UIContext {
    uuid hotID;     // probably about to be touched
    uuid activeID;  // currently being touched
    glm::vec2 mousePosition;
    bool lmouseDown;

    // std::map<uuid, std::shared_ptr<BaseState> > states;
};

uuid rootID = uuid({.owner = -1, .item = 0, .index = 0});
uuid fakeID = uuid({.owner = -2, .item = 0, .index = 0});
static std::shared_ptr<UIContext> globalContext;

std::shared_ptr<UIContext> get() {
    if (!globalContext) globalContext.reset(new UIContext());
    return globalContext;
}

void init_context() {
    get()->hotID = rootID;
    get()->activeID = rootID;
    get()->lmouseDown = false;
    get()->mousePosition = Input::getMousePosition();
}

bool isMouseInside(glm::vec4 rect) {
    auto mouseScreen = glm::vec3{Input::getMousePosition(), 0.f};
    mouseScreen.y = WIN_H - mouseScreen.y;

    auto mouse = screenToWorld(mouseScreen,                              //
                               menuCameraController->camera.view,        //
                               menuCameraController->camera.projection,  //
                               glm::vec4{0, 0, WIN_W, WIN_H});
    // log_warn("{} => {}, inside? {}", Input::getMousePosition(), mouse, rect);
    return mouse.x >= rect.x && mouse.x <= rect.x + rect.z &&
           mouse.y >= rect.y && mouse.y <= rect.y + rect.w;
}

bool button(uuid id, glm::vec2 position, glm::vec2 size) {
    bool inside =
        isMouseInside(glm::vec4{position.x, position.y, size.x, size.y});

    // everything is drawn from the center so move it so its not the center that
    // way the mouse collision works
    position.x += size.x / 2.f;
    position.y += size.y / 2.f;

    if (inside) {
        get()->hotID = id;
        if (get()->activeID == rootID && get()->lmouseDown) {
            get()->activeID = id;
        }
    }

    auto white = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    auto red = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};
    auto green = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
    auto blue = glm::vec4{0.0f, 0.0f, 1.0f, 1.0f};

    Renderer::drawQuad(position, size, white, "white");

    if (get()->hotID == id) {
        if (get()->activeID == id) {
            Renderer::drawQuad(position, size, red, "white");
        } else {
            Renderer::drawQuad(position, size, green, "white");
        }
    } else {
        Renderer::drawQuad(position, size, blue, "white");
    }

    if (!get()->lmouseDown && get()->hotID == id && get()->activeID == id) {
        return true;
    }
    return false;
}

bool slider(uuid id, glm::vec2 position, glm::vec2 size, float* value,
            float mnf, float mxf) {
    bool inside =
        isMouseInside(glm::vec4{position.x, position.y, size.x, size.y});

    float min = position.y - size.y / 2.f;
    float max = position.y + size.y / 2.f;
    float ypos = min + ((max - min) * (*value));

    if (inside) {
        get()->hotID = id;
        if (get()->activeID == rootID && get()->lmouseDown) {
            get()->activeID = id;
        }
    }

    auto white = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    auto red = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};
    auto green = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
    auto blue = glm::vec4{0.0f, 0.0f, 1.0f, 1.0f};

    auto col = white;
    if (get()->activeID == id || get()->hotID == id) {
        col = green;
    } else {
        col = blue;
    }

    // everything is drawn from the center so move it so its not the center that
    // way the mouse collision works
    auto pos =
        glm::vec2{position.x + (size.x / 2.f), position.y + (size.y / 2.f)};
    Renderer::drawQuad(pos + glm::vec2{0.f, ypos}, glm::vec2{0.5f}, col,
                       "white");
    Renderer::drawQuad(pos, size, red, "white");

    if (get()->activeID == id) {
        float v = (position.y - get()->mousePosition.y) / size.y;
        if (v < mnf) v = mnf;
        if (v > mxf) v = mxf;
        if (v != *value) {
            *value = v;
            return true;
        }
    }

    return false;
}

/*
enum UILayoutType {
    Row,
    Column,
    Grid,
};

struct FocusState {};
struct StateInfo {};

enum ControlFlags {
    can_focus = 1,
    can_press = 2,   // active when mouse / key down
    can_toggle = 4,  //
    can_select = 8,  // set on press/release pair
    can_v_scroll = 16,
    can_h_scroll = 32
};

inline ControlFlags operator|(ControlFlags a, ControlFlags b) {
    return static_cast<ControlFlags>(static_cast<int>(a) | static_cast<int>(b));
}

struct Control;
struct Control {
    std::shared_ptr<Control> parent;
    std::shared_ptr<Control> leftSibling;
    std::shared_ptr<Control> rightSibling;

    std::vector<std::shared_ptr<Control>> children;
    // sibling left  / right
    uuid id;  // should be unique among siblings in tree

    int lastFrame;
    glm::vec4 rect;
    UILayoutType layout;
    ControlFlags flags;
    FocusState focusState;
    State state;

    StateInfo stateInfo;
    std::vector<Style> styles;
};


std::shared_ptr<Control> get_or_insert(uuid id) { return nullptr; }
void update_state(std::shared_ptr<Control> control) {}
void set_parent_ptr(std::shared_ptr<Control> control) {}
std::shared_ptr<Control> get_parent_ptr() { return nullptr; }
std::shared_ptr<Control> get_parent_ptr(uuid controlID) { return nullptr; }
void set_prev_sibling_ptr(std::shared_ptr<Control> control) {}
std::shared_ptr<Control> get_prev_sibling_ptr() { return nullptr; }

bool state_active(State state) { return false; }

void start(uuid siblingID, ControlFlags controlFlags, Styles styles) {
    std::shared_ptr<Control> control = get_or_insert(siblingID);
    control->flags = controlFlags;
    control->styles = styles;

    update_state(control);

    set_parent_ptr(control);
    set_prev_sibling_ptr(nullptr);
}
void end() {
    std::shared_ptr<Control> control = get_parent_ptr();
    set_parent_ptr(control->parent);
    set_prev_sibling_ptr(control);
}

FocusState button_start(uuid siblingID, Styles styles) {
    start(siblingID, ControlFlags::can_focus | ControlFlags::can_press, styles);
    std::shared_ptr<Control> button = get_parent_ptr(siblingID);
    return button->focusState;
}
void button_end() { end(); }

bool toggle_button_start(uuid siblingID, Styles styles) {
    start(siblingID, ControlFlags::can_focus | ControlFlags::can_toggle,
          styles);
    std::shared_ptr<Control> button = get_parent_ptr(siblingID);
    return state_active(button->state);
}
void toggle_button_end() { end(); }

void find_mouse_focused(std::shared_ptr<Control> tree_root) {
    // set global active state
}
void process_input(int events) {}

void layout_start() {}
void layout_control(std::shared_ptr<Control> root) {}
void layout_end() {}
void clear_input() {}

void update() {
    int events = 0;
    // set events
    // frame start
    {
        if (!root) {
            root.reset(control_create());
        }
        find_mouse_focused(root);
        process_input(events);
        set_parent_ptr(root);
        set_prev_sibling_ptr(nullptr);
    }
    // ui
    // frame end
    // if last_Frame_idx then delete
    {
        layout_start();
        layout_control(root);
        layout_end();
        clear_input();
    }
}

*/

////// ////// ////// ////// ////// ////// ////// ////// ////// ////// //////

/*
#include <queue>

#include "event.h"

std::shared_ptr<Control> root;

std::shared_ptr<Control> get_parent_ptr() { return root; }
void set_parent_ptr(std::shared_ptr<Control> control) { root = control; }

bool is_control_root(uuid controlID) { return root->id == controlID; }
// dfs for the node in the tree with matching id
// nullptr if no matching node, or node is root
// use is_control_root to distinguish
std::shared_ptr<Control> get_parent_ptr(
    uuid controlID, std::shared_ptr<Control> parent = nullptr,
    std::shared_ptr<Control> cur = nullptr) {
    if (cur == nullptr) cur = root;
    if (cur->id == controlID) return parent;
    for (auto child : cur->children) {
        auto match = get_parent_ptr(controlID, cur, child);
        if (match) return match;
        // else nullptr
    }
    return nullptr;
}

Control* control_create() { return new Control(); }

void set_prev_sibling_ptr(std::shared_ptr<Control> control) {}
std::shared_ptr<Control> get_prev_sibling_ptr() { return nullptr; }

std::vector<Mouse::MouseButtonPressedEvent> mousePressEvents;
std::vector<KeyPressedEvent> keyPressEvents;
bool mouse_pressed(Mouse::MouseButtonPressedEvent& event) {
    // whenever a mouse is pressed
    return false;
}
bool key_pressed(KeyPressedEvent& event) {
    //
    return false;
}

void on_new_event(const Event& event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(
        std::bind(&IUI::mouse_pressed, this, std::placeholders::_1));
    dispatcher.dispatch<KeyPressedEvent>(
        std::bind(&IUI::key_pressed, this, std::placeholders::_1));
}
void clear_events() { eventsToProcess.clear(); }
void process_events() {}

void find_mouse_focused(std::shared_ptr<Control> start) {
    auto mouse = Input::getMousePosition();
    // TODO what is the mouse hovering over
}

void frame_start() {
    if (!root) {
        root.reset(control_create());
    }
    process_events();

    set_parent_ptr(root);
    set_prev_sibling_ptr(nullptr);
}
void frame_end() {
    // layout_start();
    // layout_control(root);
    // layout_end();
    clear_events();
}



struct Style {
    glm::vec4 margin;
    glm::vec4 padding;
    glm::vec4 bgColor;
    glm::vec4 borderColor;
    glm::vec4 borderWidth;
    glm::vec4 cornerRadius;

    Style(const Style& s) {
        this->margin = s.margin;
        this->padding = s.padding;
        this->bgColor = s.bgColor;
        this->borderColor = s.borderColor;
        this->borderWidth = s.borderWidth;
        this->cornerRadius = s.cornerRadius;
    }
};
typedef std::vector<Style> Styles;
struct UIContext;
template <typename T>
struct State {
   private:
    T value;

   public:
    State() {}
    State(T val) : value(val) {}
    State(const State<T>& s) : value(s.value) {}
    State<T>& operator=(const State<T>& other) {
        this->value = other.value;
        return *this;
    }
    operator T() { return value; }

    T get() { return value; }
    void set(T v) { value = v; }
};

struct BaseState {
    BaseState() {}
    virtual ~BaseState() {}
};

struct ToggleState : public BaseState {
    State<bool> on;

    ToggleState(const State<bool> o) : on(o) {}
};

struct DropdownState : public BaseState {
    State<int> selectedIndex;
    State<bool> isOpen;

    DropdownState(const State<int> index, const State<bool> open)
        : selectedIndex(index), isOpen(open) {}

    DropdownState(const DropdownState& other) {
        selectedIndex = other.selectedIndex;
        isOpen = other.isOpen;
    }
};

struct UIContext {
    uuid hotID;     // probably about to be touched
    uuid activeID;  // currently being touched

    std::map<uuid, std::shared_ptr<BaseState> > states;
};

struct TextConfig {
    std::string text;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;
};

struct ButtonConfig {
    // TODO color
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;
    TextConfig textConfig;
};

struct DropdownConfig {
    // TODO color
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;
    std::vector<TextConfig> textConfigs;
};

struct ToggleConfig {
    // TODO color
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;
};

glm::vec4 darker(glm::vec4 color, float d = 0.5) {
    auto c = color - glm::vec4{d};
    c.w = color.w;
    return c;
}


#include <type_traits>
template <typename T>
std::shared_ptr<T> getState(uuid id, T initialState) {
    if (!std::is_base_of<BaseState, T>::value) {
        log_error("Type is not a child of BaseState");
    }
    std::shared_ptr<UIContext> context = get();
    if (context->states.find(id) == context->states.end()) {
        context->states[id] = std::make_shared<T>(initialState);
    }
    return dynamic_pointer_cast<T>(context->states[id]);
}

bool active(const std::shared_ptr<UIContext> context, uuid id) {
    return context->activeID == id;
}
bool hot(const std::shared_ptr<UIContext> context, uuid id) {
    return context->hotID == id;
}
void set_active(const std::shared_ptr<UIContext> context, uuid id) {
    context->activeID = id;
}
void set_hot(const std::shared_ptr<UIContext> context, uuid id) {
    // if theres already an active item, then do nothing
    // you couldve hovered over the mouse while doing something else
    if (active(context, id)) return;
    context->hotID = id;
}
void set_not_active(const std::shared_ptr<UIContext> context, uuid id) {
    context->activeID = rootID;
}
void set_not_hot(const std::shared_ptr<UIContext> context, uuid id) {
    context->hotID = rootID;
}

bool isMouseInside(glm::vec4 rect) {
    auto mouseScreen = glm::vec3{Input::getMousePosition(), 0.f};
    mouseScreen.y = WIN_H - mouseScreen.y;

    auto mouse = screenToWorld(mouseScreen,                              //
                               menuCameraController->camera.view,        //
                               menuCameraController->camera.projection,  //
                               glm::vec4{0, 0, WIN_W, WIN_H});
    // log_warn("{} => {}, inside? {}", Input::getMousePosition(), mouse, rect);
    return mouse.x >= rect.x && mouse.x <= rect.x + rect.z &&
           mouse.y >= rect.y && mouse.y <= rect.y + rect.w;
}

bool text(const std::shared_ptr<UIContext>& context, uuid id, TextConfig config,
          glm::vec2 offset = {0.f, 0.f}) {
    int i = 0;
    for (auto c : config.text) {
        i++;
        Renderer::drawQuad(
            offset + config.position + glm::vec2{i * config.size.x, 0.f},
            config.size, config.color, std::string(1, c));
    }
    return false;
}

bool button(const std::shared_ptr<UIContext>& context, uuid id,
            ButtonConfig config) {
    bool mouseUp = Input::isMouseButtonPressed(Mouse::MouseCode::ButtonLeft);
    bool mouseDown = !mouseUp;
    bool inside = isMouseInside(glm::vec4{config.position.x, config.position.y,
                                          config.size.x, config.size.y});
    // everything is drawn from the center so move it so its not the center that
    // way the mouse collision works
    config.position.x += config.size.x / 2.f;
    config.position.y += config.size.y / 2.f;

    auto drawButton = [&](bool pressed) {
        text(context, id, config.textConfig, {0.f, 0.5f});
        Renderer::drawQuad(config.position, config.size, config.color, "white");
        if (!pressed) {
            Renderer::drawQuad(config.position + glm::vec2{0.03f, -0.03f},
                               config.size, darker(config.color), "white");
        }
    };

    // if we are active it means last frame
    // we were being clicked down
    if (active(context, id)) {
        // did we raise the mouse since last frame?
        if (mouseUp) {
            // are we still over the button?
            if (hot(context, id)) {
                drawButton(true);
                return true;
            }
            set_not_active(context, id);
        }
        // we werent clicked, but were we hovered?
    } else if (hot(context, id)) {
        // are we currently pressing the mouse?
        if (mouseDown) {
            set_active(context, id);
        }
    }
    // are we hovering over the button?
    if (inside) {
        set_hot(context, id);
    }
    drawButton(false);
    return false;
}

std::shared_ptr<ToggleState> toggle(const std::shared_ptr<UIContext>& context,
                                    uuid id,
                                    std::array<ToggleConfig, 2> configs,
                                    ToggleState initialState) {
    auto currentState = getState<ToggleState>(id, initialState);
    if (!currentState) {
        log_warn("Had matching state for ID but not correct dynamic type");
    }
    int item = 0;

    auto config = configs[currentState->on ? 1 : 0];
    bool mouseUp = Input::isMouseButtonPressed(Mouse::MouseCode::ButtonLeft);
    bool mouseDown = !mouseUp;
    bool inside = isMouseInside(glm::vec4{config.position.x, config.position.y,
                                          config.size.x, config.size.y});

    auto textConfig = TextConfig({.text = currentState->on ? "ON" : "OFF",
                                  .color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f},
                                  .position = glm::vec2{0.f, 0.f},
                                  .size = glm::vec2{0.25f, 0.25f}});

    auto buttonConfig = ButtonConfig({.textConfig = textConfig,
                                      .position = config.position,
                                      .color = config.color,
                                      .size = config.size});

    if (button(get(), uuid({id.item, item++, 0}), buttonConfig)) {
        if (mouseUp) {
            currentState->on = !currentState->on;
        }
    }

    return currentState;
}

std::shared_ptr<DropdownState> dropdown(
    const std::shared_ptr<UIContext>& context, uuid id, DropdownConfig config,
    DropdownState initialState) {
    int item = 0;
    auto currentState = getState<DropdownState>(id, initialState);
    if (!currentState) {
        log_warn("Had matching state for ID but not correct dynamic type");
    }
    return currentState;

    auto selectedTextConfig = config.textConfigs[currentState->selectedIndex];
    auto buttonConfig = ButtonConfig({
        .textConfig = selectedTextConfig,
        .color = config.color,
        .position = config.position,
        .size = config.size,
    });

    if (currentState->isOpen) {
        // todo
        if (button(get(), uuid({id.item, item++, 0}), buttonConfig)) {
            log_info("closed dropdown");
            currentState->isOpen = false;
        }
    } else {
        if (button(get(), uuid({id.item, item++, 0}), buttonConfig)) {
            log_info("opened dropdown");
            currentState->isOpen = true;
        }
    }

    return currentState;
}
*/

}  // namespace IUI
