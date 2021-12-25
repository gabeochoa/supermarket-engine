#pragma once

#include <set>
#include <string_view>

//
#include "pch.hpp"

//
#include "camera.h"
#include "font.h"
#include "input.h"
#include "renderer.h"

namespace IUI {

template <typename T>
constexpr auto type_name() {
    std::string_view name, prefix, suffix;
#ifdef __clang__
    name = __PRETTY_FUNCTION__;
    prefix = "auto type_name() [T = ";
    suffix = "]";
#elif defined(__GNUC__)
    name = __PRETTY_FUNCTION__;
    prefix = "constexpr auto type_name() [with T = ";
    suffix = "]";
#elif defined(_MSC_VER)
    name = __FUNCSIG__;
    prefix = "auto __cdecl type_name<";
    suffix = ">(void)";
#endif
    name.remove_prefix(prefix.size());
    name.remove_suffix(suffix.size());
    return name;
}

struct uuid {
    int owner;
    int item;
    int index;

    bool operator==(const uuid& other) const {
        return owner == other.owner && item == other.item &&
               index == other.index;
    }

    bool operator<(const uuid& other) const {
        if (owner < other.owner) return true;
        if (owner > other.owner) return false;
        if (item < other.item) return true;
        if (item > other.item) return false;
        if (index < other.index) return true;
        if (index > other.index) return false;
        return false;
    }
};
std::ostream& operator<<(std::ostream& os, const uuid& obj) {
    os << fmt::format("owner: {} item: {} index: {}", obj.owner, obj.item,
                      obj.index);
    return os;
}

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
    T& asT() { return value; }
    T& get() { return value; }
    void set(const T& v) { value = v; }

    // TODO how can we support += and -=?
    // is there a simple way to unfold the types,
    // tried and seeing nullptr for checkboxstate
};

// Base statetype for any UI component
struct UIState {
    virtual ~UIState() {}
};

struct CheckboxState : public UIState {
    State<bool> checked = false;
    virtual ~CheckboxState() {}
};

struct SliderState : public UIState {
    State<float> value;
};

struct TextfieldState : public UIState {
    State<std::string> buffer;
};

struct StateManager {
    std::map<uuid, std::shared_ptr<UIState>> states;

    void addState(uuid id, const std::shared_ptr<UIState>& state) {
        states[id] = state;
    }

    std::shared_ptr<UIState> getAndCreateIfNone(
        uuid id, const std::shared_ptr<UIState>& state) {
        if (states.find(id) == states.end()) {
            addState(id, state);
        }
        return get(id);
    }

    std::shared_ptr<UIState> get(uuid id) { return states[id]; }
};

// TODO Would we rather have the user specify an output
// or just let them search the UIContext?
// Something like get()->statemanager.get(uuid)

struct UIContext {
    StateManager statemanager;

    std::shared_ptr<OrthoCameraController> camController;
    uuid hotID;     // probably about to be touched
    uuid activeID;  // currently being touched

    glm::vec2 mousePosition;
    bool lmouseDown;

    const std::set<Key::KeyCode> widgetKeys = {{
        Key::mapping["Widget Next"],
        Key::mapping["Widget Press"],
        Key::mapping["Value Up"],
        Key::mapping["Value Down"],
    }};
    uuid kbFocusID;
    Key::KeyCode key;
    Key::KeyCode mod;
    uuid lastProcessed;

    const std::set<Key::KeyCode> textfieldMod = std::set<Key::KeyCode>({
        Key::mapping["Text Backspace"],
    });
    Key::KeyCode keychar;
    Key::KeyCode modchar;

    bool pressed(Key::KeyCode code) {
        bool a = key == code || mod == code;
        if (a) key = Key::KeyCode();
        return a;
    }
};

uuid rootID = uuid({.owner = -1, .item = 0, .index = 0});
uuid fakeID = uuid({.owner = -2, .item = 0, .index = 0});
static std::shared_ptr<UIContext> globalContext;

auto white = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
auto red = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};
auto green = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
auto blue = glm::vec4{0.0f, 0.0f, 1.0f, 1.0f};
auto teal = glm::vec4{0.0f, 1.0f, 1.0f, 1.0f};

inline std::shared_ptr<UIContext> get() {
    if (!globalContext) globalContext.reset(new UIContext());
    return globalContext;
}

inline void init_context() {
    get()->hotID = rootID;
    get()->activeID = rootID;
    get()->lmouseDown = false;
    get()->mousePosition = Input::getMousePosition();
}

inline void try_to_grab_kb(uuid id) {
    if (get()->kbFocusID == rootID) {
        get()->kbFocusID = id;
    }
}

inline bool has_kb_focus(uuid id) { return (get()->kbFocusID == id); }

inline void draw_if_kb_focus(uuid id, std::function<void(void)> cb) {
    if (has_kb_focus(id)) cb();
}

inline bool isMouseInside(glm::vec4 rect) {
    auto mouseScreen = glm::vec3{Input::getMousePosition(), 0.f};
    mouseScreen.y = get()->camController->camera.viewport.w - mouseScreen.y;

    auto mouse = screenToWorld(mouseScreen,                              //
                               get()->camController->camera.view,        //
                               get()->camController->camera.projection,  //
                               get()->camController->camera.viewport     //
    );
    // log_warn("{} => {}, inside? {}", Input::getMousePosition(), mouse, rect);
    return mouse.x >= rect.x && mouse.x <= rect.x + rect.z &&
           mouse.y >= rect.y && mouse.y <= rect.y + rect.w;
}

// This couldnt have been done without the tutorial
// from http://sol.gfxile.net/imgui/
// Like actually, I tried 3 times :)

struct WidgetConfig;

struct WidgetConfig {
    // const char* font = "constan";
    // const char* font = "Roboto-Regular";
    const char* font = "Karmina-Regular";
    std::string text = "";
    glm::vec2 position = glm::vec2{0.f};
    glm::vec2 size = glm::vec2{1.f};
    float rotation = 0;
    glm::vec4 color = white;
    bool transparent = false;
    std::string texture = "white";

    WidgetConfig* child;
};

template <typename T>
std::shared_ptr<T> widget_init(uuid id) {
    std::shared_ptr<UIState> gen_state =  //
        get()->statemanager.getAndCreateIfNone(id, std::make_shared<T>());
    std::shared_ptr<T> state = dynamic_pointer_cast<T>(gen_state);
    if (state == nullptr) {
        log_error(
            "State for id{} of wrong type or nullptr, expected {}. Check to "
            "make sure your id's are globally unique",
            id, type_name<T>());
    }
    return state;
}

void draw_ui_widget(glm::vec2 position, glm::vec2 size, glm::vec4 color,
                    std::string texturename, float rotation = 0.f) {
    if (rotation > 5.f) {
        Renderer::drawQuadRotated(position,                //
                                  size,                    //
                                  glm::radians(rotation),  //
                                  color,                   //
                                  texturename);
        return;
    }
    Renderer::drawQuad(position, size, color, texturename);
}

bool text(uuid id, WidgetConfig config, glm::vec2 offset = {0.f, 0.f}) {
    // NOTE: currently id is only used for focus and hot/active,
    // we could potentially also track "selections"
    // with a range so the user can highlight text
    // not needed for supermarket but could be in the future?
    (void)id;

    auto texture = fetch_texture_for_sentence(config.font, config.text.c_str());
    if (!texture) {
        log_error("failed to fetch texture for text {} with font {}",
                  config.text, config.font);
    }

    auto scaled_width = (1.f * texture->width / FONT_SIZE);
    auto scaled_height = (1.f * texture->height / FONT_SIZE);
    auto size =
        glm::vec2{config.size.x * scaled_width, config.size.y * scaled_height};

    auto position =
        // where we should draw it
        config.position +
        // local offset to align it with its parent
        offset + glm::vec2{size.x / 2.f, size.y / 2.f};

    draw_ui_widget(position, size, config.color, texture->name,
                   config.rotation);
    return false;
}

bool button(uuid id, WidgetConfig config) {
    bool inside = isMouseInside(glm::vec4{config.position, config.size});
    // everything is drawn from the center so move it so its not the center that
    // way the mouse collision works
    config.position.x += config.size.x / 2.f;
    config.position.y += config.size.y / 2.f;

    if (inside) {
        get()->hotID = id;
        if (get()->activeID == rootID && get()->lmouseDown) {
            get()->activeID = id;
        }
    }
    // if no one else has keyboard focus
    // dont mind if i do
    try_to_grab_kb(id);

    {  // start render
        if (config.text.size() != 0) {
            text(uuid({id.item, 0, 0}),
                 WidgetConfig({
                     .position = config.position,
                     .text = config.text,
                     // TODO detect if the button color is dark
                     // and change the color to white automatically
                     .color = glm::vec4{1.f - config.color.r,  //
                                        1.f - config.color.g,
                                        1.f - config.color.b, 1.f},
                     .size = glm::vec2{0.75f, 0.75f},
                 }),
                 glm::vec2{(-config.size.x / 2.f) + 0.10f, -0.75f});
        }

        draw_ui_widget(config.position, config.size, config.color,
                       config.texture, config.rotation);

        if (get()->hotID == id) {
            if (get()->activeID == id) {
                draw_ui_widget(config.position, config.size, red,
                               config.texture, config.rotation);
            } else {
                draw_ui_widget(config.position, config.size, green,
                               config.texture, config.rotation);
            }
        } else {
            draw_ui_widget(config.position, config.size, blue, config.texture,
                           config.rotation);
        }
        draw_if_kb_focus(id, [&]() {
            draw_ui_widget(config.position, config.size + glm::vec2{0.1f}, teal,
                           config.texture, config.rotation);
        });
    }  // end render
    if (has_kb_focus(id)) {
        if (get()->pressed(Key::mapping["Widget Next"])) {
            get()->kbFocusID = rootID;
            if (Input::isKeyPressed(Key::mapping["Widget Mod"])) {
                get()->kbFocusID = get()->lastProcessed;
            }
        }
    }
    // before any returns
    get()->lastProcessed = id;
    // check click
    if (has_kb_focus(id)) {
        if (get()->pressed(Key::mapping["Widget Press"])) {
            return true;
        }
    }
    if (!get()->lmouseDown && get()->hotID == id && get()->activeID == id) {
        get()->kbFocusID = id;
        return true;
    }
    return false;
}

bool button_with_label(uuid id, WidgetConfig config) {
    int item = 0;
    if (config.text == "") {
        text(uuid({id.item, item++, 0}), *config.child, config.position);
    }
    auto pressed = button(id, config);
    return pressed;
}

bool dropdown(uuid id, WidgetConfig config,
              const std::vector<WidgetConfig>& configs, bool* dropdownState,
              int* selectedIndex) {
    int item = 0;

    text(uuid({id.item, item++, 0}),
         WidgetConfig({
             .text = "^",
             .rotation = (*dropdownState) ? 0.f : 180.f,
         }),
         config.position + glm::vec2{config.size.x - 1.f, -0.25f});

    config.text = configs[*selectedIndex].text;

    if (*dropdownState) {
        float spacing = 1.0f;
        int button_item_id = item++;

        for (size_t i = 0; i < configs.size(); i++) {
            uuid button_id({id.item, button_item_id, static_cast<int>(i)});
            if (*selectedIndex == button_id.index) {
                get()->kbFocusID = button_id;
            }
            if (button_with_label(
                    button_id,
                    WidgetConfig({
                        .position = config.position +
                                    glm::vec2{0.f, -1.0f * spacing * (i + 1)},
                        .size = config.size,
                        .color = config.color,
                        .text = configs[i].text,
                    }))) {
                *selectedIndex = i;
                *dropdownState = false;
                get()->kbFocusID = id;
            }
        }

        if (Input::isKeyPressed(Key::mapping["Value Up"])) {
            (*selectedIndex) -= 1;
            if (*selectedIndex < 0) *selectedIndex = 0;
        }

        if (Input::isKeyPressed(Key::mapping["Value Down"])) {
            (*selectedIndex) += 1;
            if (*selectedIndex > (int)configs.size() - 1)
                *selectedIndex = configs.size() - 1;
        }
    }
    auto pressed = button(id, config);
    return pressed;
}

bool checkbox(uuid id, WidgetConfig config, bool* cbState = nullptr) {
    auto state = widget_init<CheckboxState>(id);
    int item = 0;

    bool changed = false;
    auto textConf = WidgetConfig({
        .text = state->checked ? "X" : " ",
        .color = glm::vec4{0.f, 0.f, 0.f, 1.f},
        .position = glm::vec2{0.1f, -0.25f},
    });
    auto conf = WidgetConfig({
        .position = config.position,  //
        .size = config.size,          //
        .child = &textConf,           //
    });
    if (button_with_label(uuid({id.item, item++, 0}), conf)) {
        state->checked = !state->checked;
        changed = true;
    }

    // If the user specified an output
    if (cbState) (*cbState) = state->checked;
    return changed;
}

bool slider(uuid id, WidgetConfig config, float* value, float mnf, float mxf) {
    auto state = widget_init<SliderState>(id);

    bool inside = isMouseInside(glm::vec4{config.position.x, config.position.y,
                                          config.size.x, config.size.y});

    float min = config.position.y - config.size.y / 2.f;
    float max = config.position.y + config.size.y / 2.f;
    float ypos = min + ((max - min) * state->value);

    if (inside) {
        get()->hotID = id;
        if (get()->activeID == rootID && get()->lmouseDown) {
            get()->activeID = id;
        }
    }

    auto col = white;
    if (get()->activeID == id || get()->hotID == id) {
        col = green;
    } else {
        col = blue;
    }

    // dont mind if i do
    try_to_grab_kb(id);

    // everything is drawn from the center so move it so its not the center
    // that way the mouse collision works
    auto pos = glm::vec2{config.position.x + (config.size.x / 2.f),
                         config.position.y + (config.size.y / 2.f)};
    draw_ui_widget(pos + glm::vec2{0.f, ypos}, glm::vec2{0.5f}, col,
                   config.texture, config.rotation);
    draw_ui_widget(pos, config.size, red, config.texture, config.rotation);

    draw_if_kb_focus(id, [&]() {
        draw_ui_widget(config.position, config.size + glm::vec2{0.1f}, teal,
                       config.texture, config.rotation);
    });

    // TODO can we have a single return statement here?
    // does it matter that the rest of the code
    // runs after changing state?

    // all drawing has to happen before this ///
    if (has_kb_focus(id)) {
        if (get()->pressed(Key::mapping["Widget Next"])) {
            get()->kbFocusID = rootID;
            if (Input::isKeyPressed(Key::mapping["Widget Mod"])) {
                get()->kbFocusID = get()->lastProcessed;
            }
        }
        if (get()->pressed(Key::mapping["Widget Press"])) {
            (*value) = state->value;
            return true;
        }
        if (Input::isKeyPressed(Key::mapping["Value Up"])) {
            state->value = state->value + 0.005;
            if (state->value > mxf) state->value = mxf;

            (*value) = state->value;
            return true;
        }
        if (Input::isKeyPressed(Key::mapping["Value Down"])) {
            state->value = state->value - 0.005;
            if (state->value < mnf) state->value = mnf;
            (*value) = state->value;
            return true;
        }
    }
    get()->lastProcessed = id;

    if (get()->activeID == id) {
        get()->kbFocusID = id;
        float v = (config.position.y - get()->mousePosition.y) / config.size.y;
        if (v < mnf) v = mnf;
        if (v > mxf) v = mxf;
        if (v != *value) {
            state->value = v;
            (*value) = state->value;
            return true;
        }
    }
    return false;
}

bool textfield(uuid id, WidgetConfig config, std::string& content) {
    auto state = widget_init<TextfieldState>(id);
    int item = 0;

    bool inside = isMouseInside(glm::vec4{config.position.x, config.position.y,
                                          config.size.x, config.size.y});

    // everything is drawn from the center so move it so its not the center
    // that way the mouse collision works
    config.position.x += config.size.x / 2.f;
    config.position.y += config.size.y / 2.f;

    if (inside) {
        get()->hotID = id;
        if (get()->activeID == rootID && get()->lmouseDown) {
            get()->activeID = id;
        }
    }

    // if no one else has keyboard focus
    // dont mind if i do
    try_to_grab_kb(id);

    {  // start render
        float tSize = 0.3f;
        auto tStartLocation =
            config.position - glm::vec2{config.size.x / 2.f, 0.f};
        auto tCursorPosition =
            tStartLocation + glm::vec2{state->buffer.asT().size() * tSize, 0.f};
        text(uuid({id.item, item++, 0}),
             WidgetConfig({.text = state->buffer,
                           .color = glm::vec4{1.0, 0.8f, 0.5f, 1.0f},
                           .position = tStartLocation,
                           .size = glm::vec2{tSize}

             }));
        draw_if_kb_focus(id, [&]() {
            text(uuid({id.item, item++, 0}),
                 WidgetConfig({.text = "_",
                               .color = glm::vec4{1.0, 0.8f, 0.5f, 1.0f},
                               .position = tCursorPosition,
                               .size = glm::vec2{tSize}

                 }));
        });
        draw_ui_widget(config.position, config.size, config.color,
                       config.texture, config.rotation);
        if (get()->hotID == id) {
            if (get()->activeID == id) {
                draw_ui_widget(config.position, config.size, red,
                               config.texture, config.rotation);
            } else {
                draw_ui_widget(config.position, config.size, green,
                               config.texture, config.rotation);
            }
        } else {
            draw_ui_widget(config.position, config.size, blue, config.texture,
                           config.rotation);
        }
        // Draw focus ring
        draw_if_kb_focus(id, [&]() {
            draw_ui_widget(config.position, config.size + glm::vec2{0.1f}, teal,
                           config.texture, config.rotation);
        });
    }  // end render

    bool changed = false;

    if (has_kb_focus(id)) {
        if (get()->pressed(Key::mapping["Widget Next"])) {
            get()->kbFocusID = rootID;
            if (Input::isKeyPressed(Key::mapping["Widget Mod"])) {
                get()->kbFocusID = get()->lastProcessed;
            }
        }
        if (get()->keychar != Key::KeyCode()) {
            state->buffer.asT().append(std::string(1, get()->keychar));
            changed = true;
        }
        if (get()->modchar == Key::mapping["Text Backspace"]) {
            state->buffer.asT().pop_back();
            changed = true;
        }
    }

    // before any returns
    get()->lastProcessed = id;

    if (!get()->lmouseDown && get()->hotID == id && get()->activeID == id) {
        get()->kbFocusID = id;
    }

    content = state->buffer;
    return changed;
}

struct UIFrame {
    UIFrame(const std::shared_ptr<OrthoCameraController> controller) {
        get()->camController = controller;
        get()->hotID = IUI::rootID;
        get()->lmouseDown =
            Input::isMouseButtonPressed(Mouse::MouseCode::ButtonLeft);
        get()->mousePosition =
            screenToWorld(glm::vec3{Input::getMousePosition(), 0.f},
                          get()->camController->camera.view,        //
                          get()->camController->camera.projection,  //
                          get()->camController->camera.viewport     //
            );
    }
    ~UIFrame() {
        if (get()->lmouseDown) {
            if (get()->activeID == IUI::rootID) {
                get()->activeID = fakeID;
            }
        } else {
            get()->activeID = IUI::rootID;
        }
        get()->key = Key::KeyCode();
        get()->mod = Key::KeyCode();

        get()->keychar = Key::KeyCode();
        get()->modchar = Key::KeyCode();
    }
};

/*
enum UILayoutType {
    Row,
    Column,
    Grid,
};

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
*/

}  // namespace IUI
