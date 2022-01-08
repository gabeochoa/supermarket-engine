#pragma once

/////////////////////////////////// ///////////////////////////////////
/*

Before using any components, you should create and init a context. this only
needs to happen once on startup, dont call it every frame.

```
    std::shared_ptr<IUI::UIContext> uicontext;
    uicontext.reset(new IUI::UIContext());
    uicontext->init();
```

To create a UI widget:
  1. call uicontext->begin to handle set for the frame
  2. create a unique uuid
  3. setup a WidgetConfig
  4. call the widget func you want to render ie `text(id, config)`
  5. call uicontext->end

Example :
```
{
    uicontext->begin(cameraController);
    if (
        button(uuid({0, 0, 0}),
               WidgetConfig({
                   .position = glm::vec2{0.f, 0.f},
                   .size = glm::vec2{2.f, 1.f}
               })
        )
    ) {
        log_info("clicked button");
    }
    uicontext->end();
}
```


1. begin(cameraController)
Handles setting up and tearing down any information needed on a frame by frame
basis.

Should be called once per frame before any of the widget functions.

TODO is this safe to be called inside itself?
something like:
{
begin();
{ begin(); end(); }
end();
}


2. UUID

When creating a UI widget, you must pass in a uuid, each widget needs a unique
id so that state and focus works correctly. Format of uuid is { parent, item,
index }



3. WidgetConfig

WidgetConfig controls all the settings for this particular widget:

WidgetConfig* child
    - if this config has a child config (only used for button_with_label)
glm::vec4 color = white;
    - for text, the font color,
    - for others, the background color
const char* font = "Karmina-Regular";
    - the font you want to use for drawing text
    - searches /resources/fonts/ for a ttf file matching this string
glm::vec2 position = glm::vec2{0.f};
    - location of the bottom left corner
float rotation = 0;
    - :)
glm::vec2 size = glm::vec2{1.f};
    - width and height of the widget
std::string text = "";
    - the text to be drawn on the ui widget
    - for button_with_label if text is empty will use child* config
std::string texture = "white";
    - the texture to use for the UI widget
    - ** not supported for text widgets
bool transparent = false;
    - whether we we should use a trasparent background texture or an opaque one
bool flipTextY = false;
    - whether we should flip the text's yscale
bool temporary = false;
    - the texture will be evicted when we generate more than MAX_TEMPORARY
string textures
    - only used for text()


3. Widget Functions

text
button (w/ label)
dropdown
checkbox
slider
textfield
commandfield
window
drawer

bool text(uuid id, WidgetConfig config);
    returns true always

bool button(uuid id, WidgetConfig config)
    returns true if the button was clicked else false


bool button_with_label(uuid id, WidgetConfig config)
    returns true if the button was clicked else false

    if config has .text will draw text directly through button() call
    if not then will check config.child for a separate text config


bool dropdown(uuid id,
              WidgetConfig config,
              // List of text configs that should show in the dropdown
              const std::vector<WidgetConfig>& configs,
              // whether or not the dropdown is open
              bool* dropdownState,
              // which item is selected
              int* selectedIndex)
    returns true if dropdown value was changed

bool checkbox(uuid id,
        WidgetConfig config,
        // whether or not the checkbox is X'd
        bool* cbState = nullptr);
    returns true if the checkbox changed


bool slider(uuid id,
            WidgetConfig config,
            // Current value of the slider
            float* value,
            // min value
            float mnf,
            // max value
            float mxf);
    returns true if slider moved


bool textfield(uuid id,
               WidgetConfig config,
               // reference to the content string the person typed in
               std::string& content);
    returns true if textfield changed

bool commandfield(uuid id, WidgetConfig config);
    returns true if command ran

bool window(uuid id, WidgetConfig config, const std::vector<WidgetConfig>&
children);
    returns true always

bool drawer(uuid id, WidgetConfig config, float* pct_open);
    returns true if fully open;


TODO add support for max-length textfield
    this will also help with temporary texture size
TODO support typing in unicode ...
    - codepoints are sent correctly from opengl already


TODO Combobox
a specific kind of dropdown that allows you to type
and it filters down to matching items

TODO ButtonGroup
TODO Do we want to add a dropdown button group?
TODO Vertical button group?

TODO Radio Buttons
TODO Toggle Button Group
- Microsoft calls this Spinbutton (for horizontal)


TODO Checkbox w/ Label
TODO Toggle Box

TODO Tabs
TODO Accordions
- this is basically a dropdown group
TODO 5 Star Rating?

TODO icons

TODO List
- usually is just a vlist of buttons
- probably can just use dropdown but always open

TODO table

TODO Transfer list?
- this is the thing with three columns that is usually used for team selection
- idk if anyone would want this instead of just implementing them yourself

TODO Drawer
- this would be pretty useful for console commands
- imagine something like fallout / skyrim console that pops down/in

TODO datepicker
TODO timezone picker?
TODO tree view
- can probably do this by indenting + accordion
TODO file treeview

TODO progress bar
- also circle

TODO color picker

TODO image carousel
TODO label carosel
TODO clock


TODO number only textfield
TODO add support for max-length textfield
this will also help with temporary texture size

TODO Auto centered text for button_with_label please
TODO value labels for sliders

TODO add support for drawing controls
TODO dropzone (drag and drop files)


TODO support different layout types
enum UILayoutType {
    Row,
    Column,
    Grid,
};

TODO support these stylings
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


State is stored in the Statemanager and you can fetch the underlying
state of a widget using IUI::get()->statemanager.get(uuid).

TODO Would we rather have the user specify an output
or just let them search the UIContext?


Util functions:

has_kb_focus(id): Lets you know if during the current frame this id is holding
keyboard focus. Must be called after the widget code has run.
    Example:
    ```
            uuid textFieldID = uuid({0, item++, 0});
            if (textfield(textFieldID, WidgetConfig({...}), content)) {
                log_info("{}", content);
            }
            if (IUI::has_kb_focus(textFieldID)) {
                editing_field = true;
            }else {
                editing_field = false;
            }
    ```

/////////////////////////////////// ///////////////////////////////////
*/

#include <set>
#include <string_view>

//
#include "edit.h"
#include "event.h"
#include "pch.hpp"
#include "typeutil.h"
#include "uuid.h"

//
#include "camera.h"
#include "font.h"
#include "input.h"
#include "renderer.h"

namespace IUI {

static const glm::vec4 white = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
static const glm::vec4 red = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};
static const glm::vec4 green = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
static const glm::vec4 blue = glm::vec4{0.0f, 0.0f, 1.0f, 1.0f};
static const glm::vec4 teal = glm::vec4{0.0f, 1.0f, 1.0f, 1.0f};

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
};

struct SliderState : public UIState {
    State<float> value;
};

struct TextfieldState : public UIState {
    State<std::wstring> buffer;
};

struct ToggleState : public UIState {
    State<bool> on;
};

struct DropdownState : public ToggleState {
    State<int> selected;
};

struct DrawerState : public ToggleState {
    State<float> heightPct;
};

struct PlusMinusButtonState : public UIState {
    State<float> value;
};

struct StateManager {
    std::map<uuid, std::shared_ptr<UIState>> states;

    void addState(uuid id, const std::shared_ptr<UIState>& state) {
        states[id] = state;
    }

    template <typename T>
    std::shared_ptr<T> getAndCreateIfNone(uuid id) {
        if (states.find(id) == states.end()) {
            addState(id, std::make_shared<T>());
        }
        return get_as<T>(id);
    }

    std::shared_ptr<UIState> get(uuid id) { return states[id]; }

    template <typename T>
    std::shared_ptr<T> get_as(uuid id) {
        try {
            return dynamic_pointer_cast<T>(states.at(id));
        } catch (std::exception) {
            return nullptr;
        }
    }
};

struct UIContext;
static UIContext* globalContext;
inline UIContext* get() {
    if (!globalContext) {
        log_error("{}", "Trying to grab context but you dont have one active");
    }
    return globalContext;
}

struct UIContext {
    int c_id = 0;
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

    bool processCharPressEvent(CharPressedEvent& event) {
        keychar = static_cast<Key::KeyCode>(event.charcode);
        return true;
    }

    bool processKeyPressEvent(KeyPressedEvent& event) {
        if (widgetKeys.count(static_cast<Key::KeyCode>(event.keycode)) == 1) {
            key = static_cast<Key::KeyCode>(event.keycode);
        }
        if (event.keycode == Key::mapping["Widget Mod"]) {
            mod = static_cast<Key::KeyCode>(event.keycode);
        }
        if (textfieldMod.count(static_cast<Key::KeyCode>(event.keycode)) == 1) {
            modchar = static_cast<Key::KeyCode>(event.keycode);
        }
        return false;
    }

    std::function<bool(CharPressedEvent&)> getCharPressHandler() {
        return std::bind(&UIContext::processCharPressEvent, this,
                         std::placeholders::_1);
    }

    void init() {
        hotID = rootID;
        activeID = rootID;
        lmouseDown = false;
        mousePosition = Input::getMousePosition();
    }

    void begin(const std::shared_ptr<OrthoCameraController> controller) {
        globalContext = this;
        camController = controller;
        hotID = rootID;
        lmouseDown = Input::isMouseButtonPressed(Mouse::MouseCode::ButtonLeft);
        mousePosition = screenToWorld(glm::vec3{Input::getMousePosition(), 0.f},
                                      camController->camera.view,        //
                                      camController->camera.projection,  //
                                      camController->camera.viewport     //
        );
    }
    void end() {
        if (lmouseDown) {
            if (activeID == rootID) {
                activeID = fakeID;
            }
        } else {
            activeID = IUI::rootID;
        }
        key = Key::KeyCode();
        mod = Key::KeyCode();

        keychar = Key::KeyCode();
        modchar = Key::KeyCode();
        globalContext = nullptr;
    }
};

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

// if you need a non english font, grab it from here
// https://fonts.google.com/noto/fonts

struct WidgetConfig;

struct WidgetConfig {
    WidgetConfig* child;
    glm::vec4 color = white;
    // const char* font = "constan";
    // const char* font = "Roboto-Regular";
    const char* font = "Karmina-Regular";
    glm::vec2 position = glm::vec2{0.f};
    float rotation = 0;
    glm::vec2 size = glm::vec2{1.f};
    std::string text = "";
    std::string texture = "white";
    bool transparent = false;
    bool vertical = false;
    bool flipTextY = false;
    bool temporary = false;
};

template <typename T>
std::shared_ptr<T> widget_init(uuid id) {
    std::shared_ptr<T> state = get()->statemanager.getAndCreateIfNone<T>(id);
    if (state == nullptr) {
        log_error(
            "State for id ({}) of wrong type, expected {}. Check to "
            "make sure your id's are globally unique",
            id, type_name<T>());
    }
    return state;
}

inline void draw_ui_widget(glm::vec2 position, glm::vec2 size, glm::vec4 color,
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

bool text(uuid id, WidgetConfig config) {
    // NOTE: currently id is only used for focus and hot/active,
    // we could potentially also track "selections"
    // with a range so the user can highlight text
    // not needed for supermarket but could be in the future?
    (void)id;

    std::shared_ptr<Texture> texture;
    texture = fetch_texture_for_phrase(config.font, to_wstring(config.text),
                                       config.temporary);
    if (!texture) {
        log_error("failed to fetch texture for text {} with font {}",
                  config.text, config.font);
    }

    auto scaled_width = (1.f * texture->width / FONT_SIZE);
    auto scaled_height = (1.f * texture->height / FONT_SIZE);
    auto size =
        glm::vec2{config.size.x * scaled_width,
                  (config.flipTextY ? -1 : 1) * config.size.y * scaled_height};

    auto position =
        // where we should draw it
        config.position +
        //
        glm::vec2{size.x / 2.f, size.y / 2.f};

    draw_ui_widget(position, size, config.color, texture->name,
                   config.rotation);
    return true;
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

        draw_if_kb_focus(id, [&]() {
            draw_ui_widget(config.position, config.size + glm::vec2{0.1f}, teal,
                           config.texture, config.rotation);
        });

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
        draw_ui_widget(config.position, config.size, config.color,
                       config.texture, config.rotation);

        if (config.text.size() != 0) {
            float sign = config.flipTextY ? 1 : -1;

            WidgetConfig textConfig(config);
            // TODO detect if the button color is dark
            // and change the color to white automatically
            textConfig.color =
                glm::vec4{1.f - config.color.r,  //
                          1.f - config.color.g, 1.f - config.color.b, 1.f};
            textConfig.position =
                config.position + glm::vec2{(-config.size.x / 2.f) + 0.10f,
                                            config.size.y * sign * 0.5f};
            textConfig.size = 0.75f * glm::vec2{config.size.y, config.size.y};
            text(uuid({id.item, 0, 0}), textConfig);
        }

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
    auto pressed = button(id, config);
    if (config.text == "") {
        // apply offset so text is relative to button position
        config.child->position += config.position;
        text(uuid({id.item, item++, 0}), *config.child);
    }
    return pressed;
}

bool dropdown(uuid id, WidgetConfig config,
              const std::vector<WidgetConfig>& configs, bool* dropdownState,
              int* selectedIndex) {
    auto state = widget_init<DropdownState>(id);
    if (dropdownState) state->on.set(*dropdownState);
    if (selectedIndex) state->selected.set(*selectedIndex);

    int item = 0;

    config.text = configs[state->selected].text;

    if (state->on) {
        float spacing = config.size.y * 1.0f;
        float sign = config.flipTextY ? 1.f : -1.f;
        int button_item_id = item++;

        for (size_t i = 0; i < configs.size(); i++) {
            uuid button_id({id.item, button_item_id, static_cast<int>(i)});
            if (*selectedIndex == button_id.index) {
                get()->kbFocusID = button_id;
            }
            if (button_with_label(
                    button_id,
                    WidgetConfig({
                        .color = config.color,
                        .position = config.position +
                                    glm::vec2{0.f, sign * spacing * (i + 1)},
                        .size = config.size,
                        .text = configs[i].text,
                        .flipTextY = config.flipTextY,
                    }))) {
                state->selected = i;
                state->on = false;
                get()->kbFocusID = id;
            }
        }

        if (get()->pressed(Key::mapping["Value Up"])) {
            state->selected = state->selected - 1;
            if (state->selected < 0) state->selected = 0;
        }

        if (get()->pressed(Key::mapping["Value Down"])) {
            state->selected = state->selected + 1;
            if (state->selected > (int)configs.size() - 1)
                state->selected = configs.size() - 1;
        }
    }
    auto pressed = button(id, config);

    // TODO rotation is not really working correctly and so we have to
    // offset the V a little more than ^ in order to make it look nice
    auto offset = glm::vec2{config.size.x - (state->on ? 1.f : 1.6f),
                            config.size.y * -0.25f};
    text(uuid({id.item, item++, 0}),
         WidgetConfig({.rotation = state->on ? 90.f : 270.f,
                       .text = ">",
                       .flipTextY = config.flipTextY,
                       .position = config.position + offset}));

    auto ret = *dropdownState != state->on.asT() ||
               *selectedIndex != state->selected.asT() ||
               (pressed && state->on.asT());
    if (pressed) {
        state->on = !state->on;
    }

    if (dropdownState) *dropdownState = state->on;
    if (selectedIndex) *selectedIndex = state->selected;

    return ret;
}

bool checkbox(uuid id, WidgetConfig config, bool* cbState = nullptr) {
    auto state = widget_init<CheckboxState>(id);
    if (cbState) state->checked.set(*cbState);

    int item = 0;

    bool changed = false;
    auto textConf = WidgetConfig({
        .color = glm::vec4{0.f, 0.f, 0.f, 1.f},
        .position = glm::vec2{0.1f, -0.25f},
        .text = state->checked ? "X" : "",
    });
    auto conf = WidgetConfig({
        .child = &textConf,           //
        .position = config.position,  //
        .size = config.size,          //
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
    if (value) state->value.set(*value);

    bool inside = isMouseInside(glm::vec4{config.position.x, config.position.y,
                                          config.size.x, config.size.y});

    float min;
    float max;
    if (config.vertical) {
        min = config.position.y - (config.size.y / 2.f);
        max = config.position.y + (config.size.y / 2.f);
    } else {
        min = config.position.x - (config.size.x / 2.f);
        max = config.position.x + (config.size.x / 2.f);
    }
    float pos_offset = ((max - min) * state->value);

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

    draw_if_kb_focus(id, [&]() {
        draw_ui_widget(pos, config.size + glm::vec2{0.1f}, teal, config.texture,
                       config.rotation);
    });

    // slider rail
    draw_ui_widget(pos, config.size, red, config.texture, config.rotation);

    // slide
    if (config.vertical) {
        draw_ui_widget(
            config.position + glm::vec2{config.size.x / 2.f, pos_offset},
            glm::vec2{0.5f}, col, config.texture, config.rotation);
    } else {
        draw_ui_widget(
            config.position + glm::vec2{pos_offset, config.size.y / 2.f},
            glm::vec2{0.5f}, col, config.texture, config.rotation);
    }

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
        float v;
        if (config.vertical) {
            v = (config.position.y - get()->mousePosition.y) / config.size.y;
        } else {
            v = (get()->mousePosition.x - config.position.x) / config.size.x;
        }
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

// TODO add support for max-length textfield
// this will also help with temporary texture size

bool textfield(uuid id, WidgetConfig config, std::wstring& content) {
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

        // Draw focus ring
        draw_if_kb_focus(id, [&]() {
            draw_ui_widget(config.position, config.size + glm::vec2{0.1f}, teal,
                           config.texture, config.rotation);
        });

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
        draw_ui_widget(config.position, config.size, config.color,
                       config.texture, config.rotation);

        float tSize = config.size.y * 0.5f;
        auto tStartLocation =
            config.position -
            glm::vec2{config.size.x / 2.f,
                      // only move text if not flipped otherwise
                      // text will be too high in the box
                      config.flipTextY ? -tSize : 0.5f};

        std::wstring focusStr = has_kb_focus(id) ? L"_" : L"";
        std::wstring focused_content =
            fmt::format(L"{}{}", state->buffer.asT(), focusStr);

        text(uuid({id.item, item++, 0}),
             WidgetConfig({
                 .color = glm::vec4{1.0, 0.8f, 0.5f, 1.0f},
                 .position = tStartLocation,
                 .size = glm::vec2{tSize},
                 .text = to_string(focused_content),
                 .flipTextY = config.flipTextY,
                 .temporary = true,
             }));

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
            state->buffer.asT().append(std::wstring(1, get()->keychar));
            changed = true;
        }
        if (get()->modchar == Key::mapping["Text Backspace"]) {
            if (state->buffer.asT().size() > 0) {
                state->buffer.asT().pop_back();
            }
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

bool commandfield(uuid id, WidgetConfig config, std::wstring& content) {
    // We dont need a separate ID since we want
    // global state to match and for them to have the
    // same keyboard focus
    textfield(id, config, content);

    if (has_kb_focus(id)) {
        // TODO this could totally throw and you know it
        auto state = get()->statemanager.get_as<TextfieldState>(id);
        if (get()->pressed(Key::mapping["Command Enter"])) {
            // Any concerned about non english characters?
            EDITOR_COMMANDS.triggerCommand(to_string(state->buffer.asT()));
            state->buffer.asT().clear();
            return true;
        }
        // TODO how to do tab completion without tab access ?
        // TODO probably make a separate mapping for this
        if (get()->pressed(Key::mapping["Value Up"])) {
            state->buffer.asT() =
                to_wstring(EDITOR_COMMANDS.command_history.back());
        }
        if (get()->pressed(Key::mapping["Value Down"])) {
            state->buffer.asT().clear();
        }
    }
    // TODO what should we return here,
    // changed? or user ran command
    return false;
}

bool window(uuid, WidgetConfig config) {
    draw_ui_widget(config.position, config.size, config.color, config.texture,
                   config.rotation);
    return true;
}

bool drawer(uuid id, WidgetConfig config, float* pct_open = nullptr) {
    auto state = widget_init<DrawerState>(id);
    if (pct_open) state->heightPct = *pct_open;

    if (state->heightPct < 1.f) {
        state->heightPct.asT() += 0.1;
    }
    draw_ui_widget(
        glm::vec2{config.position.x, state->heightPct * config.position.y},
        config.size, config.color, config.texture, config.rotation);
    // output pct if user wants
    if (pct_open) *pct_open = state->heightPct;
    // return if drawer should render children
    if (state->heightPct > 0.9) return true;
    return false;
}

// TODO not ready for use, need to mess around with the location / scales
// some more
inline bool plusMinusButton(uuid id, WidgetConfig config,
                            float* value = nullptr, float increment = 0.1) {
    auto state = widget_init<PlusMinusButtonState>(id);
    if (value) state->value = *value;
    int item = 0;
    bool changed = false;

    WidgetConfig original(config);

    config.size /= 2.f;
    config.position += glm::vec2{0, +config.size.y};
    config.text = ">";
    config.rotation = 270.f;
    if (button(uuid({id.owner, id.item, item++}), config)) {
        state->value = state->value + increment;
        changed = true;
    }

    config.rotation = 90.f;
    config.position.y -= config.size.y * 2;
    if (button(uuid({id.owner, id.item, item++}), config)) {
        state->value = state->value + increment;
        changed = true;
    }

    original.position.x += config.size.x;
    text(uuid({id.owner, id.item, item++}), original);

    // TODO allow support for setting these
    state->value = fmaxf(0, state->value);
    state->value = fminf(9999, state->value);

    if (value) *value = state->value;
    return changed;
}

}  // namespace IUI
