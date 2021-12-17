
#pragma once

#include "input.h"
#include "pch.hpp"

namespace IUI {

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

struct uuid {
    int owner;
    int item;
    int index;

    bool operator==(const uuid& other) const {
        return owner == other.owner && item == other.item &&
               index == other.index;
    }
};

enum UILayoutType {
    Row,
    Column,
    Grid,
};

struct FocusState {};
struct State {};
struct StateInfo {};

enum ControlFlags {
    can_focus = 1,
    can_press = 2,  // active when mouse / key down
    can_toggle = 4,
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

/*

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

*/

//
//
//
//
//
//
//
//
//
//
//
//
//
//

struct UIContext {
    uuid hotID;     // probably about to be touched
    uuid activeID;  // currently being touched
};

uuid rootID;
static std::shared_ptr<UIContext> globalContext;

std::shared_ptr<UIContext> get() {
    if (!globalContext) globalContext.reset(new UIContext());
    return globalContext;
}

bool active(UIContext context, uuid id) { return context.activeID == id; }
bool hot(UIContext context, uuid id) { return context.hotID == id; }
void set_active(UIContext context, uuid id) { context.activeID = id; }
void set_hot(UIContext context, uuid id) {
    // if theres already an active item, then do nothing
    // you couldve hovered over the mouse while doing something else
    if (active(context, id)) return;
    context.hotID = id;
}
void set_not_active(UIContext context, uuid id) { context.activeID = rootID; }
void set_not_hot(UIContext context, uuid id) { context.hotID = rootID; }

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

bool text(UIContext context, uuid id, TextConfig config) {
    int i = 0;
    for (auto c : config.text) {
        i++;
        Renderer::drawQuad(config.position + glm::vec2{i * 1.5f, 0.f},
                           config.size, config.color, std::string(1, c));
    }
    return false;
}

bool button(UIContext context, uuid id, ButtonConfig config) {
    bool mouseUp = Input::isMouseButtonPressed(Mouse::MouseCode::ButtonLeft);
    bool mouseDown = !mouseUp;
    bool inside = false;

    auto darker = [](glm::vec4 color, float d = 0.5) {
        auto c = color - glm::vec4{d};
        c.w = color.w;
        return c;
    };

    auto drawButton = [&]() {
        // Renderer::drawQuad(config.position, config.size, config.color,
        // "white"); Renderer::drawQuad(config.position + glm::vec2{0.03f,
        // -0.03f}, config.size, darker(config.color), "white");
        text(context, id, config.textConfig);
    };

    if (active(context, id)) {
        if (mouseUp) {
            if (hot(context, id)) {
                drawButton();
                return true;
            }
            set_not_active(context, id);
        }
    } else if (hot(context, id)) {
        if (mouseDown) {
            set_active(context, id);
        }
    }
    if (inside) {
        set_hot(context, id);
    }
    drawButton();
    return false;
}

}  // namespace IUI
