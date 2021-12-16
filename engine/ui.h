
#pragma once

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

std::shared_ptr<Control> iui_root;

std::shared_ptr<Control> iui_get_or_insert(uuid id) { return nullptr; }
void iui_update_state(std::shared_ptr<Control> control) {}
void iui_set_parent_ptr(std::shared_ptr<Control> control) {}
std::shared_ptr<Control> iui_get_parent_ptr() { return nullptr; }
std::shared_ptr<Control> iui_get_parent_ptr(uuid controlID) { return nullptr; }
void iui_set_prev_sibling_ptr(std::shared_ptr<Control> control) {}
std::shared_ptr<Control> iui_get_prev_sibling_ptr() { return nullptr; }

bool iui_state_active(State state) { return false; }

void iui_start(uuid siblingID, ControlFlags controlFlags, Styles styles) {
    std::shared_ptr<Control> control = iui_get_or_insert(siblingID);
    control->flags = controlFlags;
    control->styles = styles;

    iui_update_state(control);

    iui_set_parent_ptr(control);
    iui_set_prev_sibling_ptr(nullptr);
}
void iui_end() {
    std::shared_ptr<Control> control = iui_get_parent_ptr();
    iui_set_parent_ptr(control->parent);
    iui_set_prev_sibling_ptr(control);
}

FocusState iui_button_start(uuid siblingID, Styles styles) {
    iui_start(siblingID, ControlFlags::can_focus | ControlFlags::can_press,
              styles);
    std::shared_ptr<Control> button = iui_get_parent_ptr(siblingID);
    return button->focusState;
}
void iui_button_end() { iui_end(); }

bool iui_toggle_button_start(uuid siblingID, Styles styles) {
    iui_start(siblingID, ControlFlags::can_focus | ControlFlags::can_toggle,
              styles);
    std::shared_ptr<Control> button = iui_get_parent_ptr(siblingID);
    return iui_state_active(button->state);
}
void iui_toggle_button_end() { iui_end(); }

Control* iui_control_create() { return nullptr; }
void iui_find_mouse_focused(std::shared_ptr<Control> tree_root) {
    // set global active state
}
void iui_process_input(int events) {}

void iui_layout_start() {}
void iui_layout_control(std::shared_ptr<Control> iui_root) {}
void iui_layout_end() {}
void iui_clear_input() {}

void iui_update() {
    int events = 0;
    // set events
    // frame start
    {
        if (!iui_root) {
            iui_root.reset(iui_control_create());
        }
        iui_find_mouse_focused(iui_root);
        iui_process_input(events);
        iui_set_parent_ptr(iui_root);
        iui_set_prev_sibling_ptr(nullptr);
    }
    // ui
    // frame end
    // if last_Frame_idx then delete
    {
        iui_layout_start();
        iui_layout_control(iui_root);
        iui_layout_end();
        iui_clear_input();
    }
}

//
///
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

bool active(UIContext context, uuid id) { return context.activeID == id; }
bool hot(UIContext context, uuid id) { return context.hotID == id; }

void set_active(UIContext context, uuid id) {}
void set_hot(UIContext context, uuid id) {
    // if theres already an active item, then do nothing
    // you couldve hovered over the mouse while doing something else
}
void set_not_active(UIContext context, uuid id) {}
void set_not_hot(UIContext context, uuid id) {}

bool button(UIContext context, uuid id, const char* text) {
    bool mouseUp = false;
    bool mouseDown = false;
    bool inside = false;

    if (active(context, id)) {
        if (mouseUp) {
            if (hot(context, id)) {
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
    // draw button
    return false;
}

}  // namespace IUI
