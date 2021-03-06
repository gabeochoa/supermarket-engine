#pragma once

/////////////////////////////////// ///////////////////////////////////
/*

Before using any components, you should create and init a context. this only
needs to happen once on startup, dont call it every frame.

```
    std::shared_ptr<GOUI::UIContext> uicontext;
    uicontext.reset(new GOUI::UIContext());
    uicontext->init();
```

// TODO: if you have two functions
// render(){ ui1(); ui2();}
// calling begin()/end() in both ui1 and ui2 breaks active/hot since its a
shared context
// so instead you have to render(){ begin(); ui1(); ui2(); end();}
//
// Is this what we want?
//

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
        button(MK_UUID(0),
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
id so that state and focus works correctly. Format of uuid is { parent, item },
but the item is statically generated so all you need to pass in is the current
layer



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
button_list
dropdown
checkbox
slider
textfield
commandfield
window
drawer

bool text(const uuid id, WidgetConfig config);
    returns true always

bool button(const uuid id, WidgetConfig config)
    returns true if the button was clicked else false


bool button_with_label(const uuid id, WidgetConfig config)
    returns true if the button was clicked else false

    if config has .text will draw text directly through button() call
    if not then will check config.child for a separate text config


bool button_list(const uuid id,
              WidgetConfig config,
              // List of text configs that should show in the dropdown
              const std::vector<WidgetConfig>& configs,
              // which item is selected
              int* selectedIndex)
TODO is this what we want?
    returns true if any button pressed

bool dropdown(const uuid id,
              WidgetConfig config,
              // List of text configs that should show in the dropdown
              const std::vector<WidgetConfig>& configs,
              // whether or not the dropdown is open
              bool* dropdownState,
              // which item is selected
              int* selectedIndex)
    returns true if dropdown value was changed

bool checkbox(const uuid id,
        WidgetConfig config,
        // whether or not the checkbox is X'd
        bool* cbState = nullptr);
    returns true if the checkbox changed


bool slider(const uuid id,
            WidgetConfig config,
            // Current value of the slider
            float* value,
            // min value
            float mnf,
            // max value
            float mxf);
    returns true if slider moved

bool textfield(const uuid id,
               WidgetConfig config,
               // reference to the content string the person typed in
               std::string& content);
    returns true if textfield changed

bool commandfield(const uuid id, WidgetConfig config);
    returns true if command ran

bool window(const uuid id, WidgetConfig config, const std::vector<WidgetConfig>&
children);
    returns true always

bool drawer(const uuid id, WidgetConfig config, float* pct_open);
    returns true if fully open;


TODO add TextGroups
    - where it draws the second after the first
    - hspacing / v spacing
    - meta? header? body?

TODO add popup / hover / tooltip

TODO add button on press visual difference
    - we kinda have this on uitest but not for normal buttons

TODO add support for max-length textfield
    this will also help with temporary texture size
TODO support typing in unicode ...
    - codepoints are sent correctly from opengl already


TODO Combobox
a specific kind of dropdown that allows you to type
and it filters down to matching items

TODO ButtonGroup

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
state of a widget using GOUI::get()->statemanager.get(uuid).

TODO Would we rather have the user specify an output
or just let them search the UIContext?


Util functions:

has_kb_focus(id): Lets you know if during the current frame this id is holding
keyboard focus. Must be called after the widget code has run.
    Example:
    ```
            uuid textFieldID = MK_UUID(0);
            if (textfield(textFieldID, WidgetConfig({...}), content)) {
                log_info("{}", content);
            }
            if (GOUI::has_kb_focus(textFieldID)) {
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
#include "log.h"
#include "strutil.h"
#include "typeutil.h"
#include "uuid.h"

namespace GOUI {

static const glm::vec4 DEFAULT_COLOR = glm::vec4{-1.0f, 1.0f, 1.0f, 1.0f};
static const glm::vec4 white = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
static const glm::vec4 black = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
static const glm::vec4 red = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};
static const glm::vec4 green = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
static const glm::vec4 blue = glm::vec4{0.0f, 0.0f, 1.0f, 1.0f};
static const glm::vec4 teal = glm::vec4{0.0f, 1.0f, 1.0f, 1.0f};
static const glm::vec4 magenta = glm::vec4{1.0f, 0.0f, 1.0f, 1.0f};

inline glm::vec4 getOppositeColor(const glm::vec4& color) {
    // TODO detect if the button color is dark
    // and change the color to white automatically
    return glm::vec4{1.f - color.r, 1.f - color.g, 1.f - color.b, color.a};
}

struct WidgetTheme {
    enum ColorType {
        FONT = 0,
        FG = 0,
        BG,
    };

    glm::vec4 fontColor = DEFAULT_COLOR;
    glm::vec4 backgroundColor = DEFAULT_COLOR;
    std::string texture = "white";

    glm::vec4 color(WidgetTheme::ColorType type = ColorType::BG) const {
        if (type == ColorType::FONT || type == ColorType::FG) {
            if (fontColor.x < 0)  // default
                return black;
            return fontColor;
        }
        if (backgroundColor.x < 0)  // default
            return white;
        return backgroundColor;
    }
};

template <typename T>
struct State {
   private:
    T value = T();

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
    State<int> cursorBlinkTime;
    State<bool> showCursor;
};

struct CommandfieldState : public TextfieldState {
    State<std::vector<std::string>> autocomp;
    State<int> selected = State<int>(-1);
};

struct ButtonListState : public UIState {
    State<int> selected;
    State<bool> hasFocus;
};

struct ToggleState : public UIState {
    State<bool> on;
};

struct DropdownState : public ToggleState {};

struct DrawerState : public ToggleState {
    State<float> heightPct;
};

struct ScrollViewState : public UIState {
    State<float> yoffset;
};

struct PlusMinusButtonState : public UIState {
    State<float> value;
};

struct StateManager {
    std::map<uuid, std::shared_ptr<UIState>> states;

    void addState(const uuid id, const std::shared_ptr<UIState>& state) {
        states[id] = state;
    }

    template <typename T>
    std::shared_ptr<T> getAndCreateIfNone(const uuid id) {
        if (states.find(id) == states.end()) {
            addState(id, std::make_shared<T>());
        }
        return get_as<T>(id);
    }

    std::shared_ptr<UIState> get(const uuid id) { return states[id]; }

    template <typename T>
    std::shared_ptr<T> get_as(const uuid id) {
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

// TODO ifdef on debug ?
#define SUPERMARKET_HOUSEKEEPING 1

struct UIContext {
#ifdef SUPERMARKET_HOUSEKEEPING
    // house keeping :)
    bool inited = false;
    bool began_and_not_ended = false;
    bool inited_keys = false;
#endif

    int c_id = 0;
    StateManager statemanager;

    uuid hotID;     // probably about to be touched
    uuid activeID;  // currently being touched

    glm::vec2 mousePosition;
    bool lmouseDown;

    std::map<std::string, int> keyMapping;

    std::set<int> widgetKeys;
    uuid kbFocusID;
    int key;
    int mod;
    uuid lastProcessed;

    std::set<int> textfieldMods;
    std::set<int> widgetMods;
    int keychar;
    int modchar;

    bool pressed(int code) {
        bool a = pressedWithoutEat(code);
        if (a) eatKey();
        return a;
    }

    void eatKey() { key = int(); }

    bool pressedWithoutEat(int code) const {
        return key == code || mod == code;
    }

    bool processCharPressEvent(int charcode) {
        keychar = charcode;
        return true;
    }

    bool processKeyPressEvent(int keycode) {
        if (widgetKeys.count(keycode) == 1) {
            key = keycode;
        }
        if (widgetMods.count(keycode) == 1) {
            mod = keycode;
        }
        if (textfieldMods.count(keycode) == 1) {
            modchar = keycode;
        }
        return false;
    }

    float yscrolled;
    bool processMouseScrolled(float yoffset) {
        yscrolled = yoffset;
        return true;
    }

    std::function<bool(glm::vec4)> isMouseInside;
    std::function<bool(int)> isKeyPressed;
    std::function<void(glm::vec2, glm::vec2, float, glm::vec4, std::string)>
        drawWidget;

    struct FontPhraseTexInfo {
        std::string textureName;
        int width;
        int height;
        int fontSize;

        bool valid() {
            return fontSize != 0 && width != 0 && height != 0 &&
                   !textureName.empty();
        }
    };
    typedef std::function<FontPhraseTexInfo(std::string, std::wstring, bool)>
        GenFontPhraseTexture;

    GenFontPhraseTexture generateFontPhraseTexture;

    struct KeyCodes {
        int widgetNext = 258;        // tab
        int widgetPress = 257;       // enter
        int widgetMod = 340;         // shift
        int valueUp = 265;           // up arrow
        int valueDown = 264;         // down arrow
        int textBackspace = 259;     // backspace
        int commandEnter = 257;      // enter
        int clearCommandLine = 261;  // delete
    };

    void init_keys(KeyCodes keycodes) {
        keyMapping["Widget Next"] = keycodes.widgetNext;
        keyMapping["Widget Press"] = keycodes.widgetPress;
        keyMapping["Widget Mod"] = keycodes.widgetMod;
        keyMapping["Value Up"] = keycodes.valueUp;
        keyMapping["Value Down"] = keycodes.valueDown;
        keyMapping["Text Backspace"] = keycodes.textBackspace;
        keyMapping["Command Enter"] = keycodes.commandEnter;
        keyMapping["Clear Command Line"] = keycodes.clearCommandLine;

        widgetMods.insert(keycodes.widgetMod);

        widgetKeys.insert(keycodes.widgetNext);
        widgetKeys.insert(keycodes.widgetPress);
        widgetKeys.insert(keycodes.valueUp);
        widgetKeys.insert(keycodes.valueDown);
        widgetKeys.insert(keycodes.commandEnter);
        widgetKeys.insert(keycodes.clearCommandLine);

        textfieldMods.insert(keycodes.textBackspace);

#ifdef SUPERMARKET_HOUSEKEEPING
        inited_keys = true;
#endif
    }

    void init(
        std::function<bool(glm::vec4)> mouseInsideFn,
        std::function<bool(int)> isKeyPressedFn,
        std::function<void(glm::vec2, glm::vec2, float, glm::vec4, std::string)>
            drawFn,
        GenFontPhraseTexture fetchTexForPhraseFn) {
#ifdef SUPERMARKET_HOUSEKEEPING
        inited = true;
        began_and_not_ended = false;
#endif
        hotID = rootID;
        activeID = rootID;
        kbFocusID = rootID;
        lmouseDown = false;
        mousePosition = glm::vec2{0};
        isMouseInside = mouseInsideFn;
        isKeyPressed = isKeyPressedFn;
        drawWidget = drawFn;
        generateFontPhraseTexture = fetchTexForPhraseFn;
    }

    void begin(bool mouseDown, glm::vec2 mousePos) {
#ifdef SUPERMARKET_HOUSEKEEPING
        M_ASSERT(inited, "UIContext must be inited before you begin()");
        M_ASSERT(!began_and_not_ended,
                 "You should call end every frame before calling begin() "
                 "again ");
        began_and_not_ended = true;
        M_ASSERT(inited_keys,
                 "UIContext init_keys() must be inited before you begin()");
#endif

        globalContext = this;
        hotID = rootID;
        lmouseDown = mouseDown;
        mousePosition = mousePos;
    }

    void end() {
#ifdef SUPERMARKET_HOUSEKEEPING
        began_and_not_ended = false;
#endif
        if (lmouseDown) {
            if (activeID == rootID) {
                activeID = fakeID;
            }
        } else {
            activeID = rootID;
        }
        key = int();
        mod = int();

        keychar = int();
        modchar = int();
        globalContext = nullptr;
    }
};

inline bool isHot(const uuid& id) { return (get()->hotID == id); }
inline bool isActive(const uuid& id) { return (get()->activeID == id); }
inline bool isActiveOrHot(const uuid& id) { return isHot(id) || isActive(id); }
inline bool isActiveAndHot(const uuid& id) { return isHot(id) && isActive(id); }

inline void try_to_grab_kb(const uuid id) {
    if (get()->kbFocusID == rootID) {
        get()->kbFocusID = id;
    }
}

inline bool has_kb_focus(const uuid& id) { return (get()->kbFocusID == id); }

inline void draw_if_kb_focus(const uuid& id, std::function<void(void)> cb) {
    if (has_kb_focus(id)) cb();
}

// This couldnt have been done without the tutorial
// from http://sol.gfxile.net/imgui/
// Like actually, I tried 3 times :)

// if you need a non english font, grab it from here
// https://fonts.google.com/noto/fonts

struct WidgetConfig;

struct WidgetConfig {
    WidgetConfig* child;
    WidgetTheme theme;
    const char* font = "default";
    glm::vec2 position = glm::vec2{0.f};
    float rotation = 0;
    glm::vec2 size = glm::vec2{1.f};
    std::string text = "";
    bool transparent = false;
    bool vertical = false;
    bool flipTextY = false;
    bool temporary = false;
};

typedef std::function<void(WidgetConfig)> Child;

template <typename T>
std::shared_ptr<T> widget_init(const uuid id) {
    std::shared_ptr<T> state = get()->statemanager.getAndCreateIfNone<T>(id);
    if (state == nullptr) {
        log_error(
            "State for id ({}) of wrong type, expected {}. Check to "
            "make sure your id's are globally unique",
            std::string(id), type_name<T>());
    }
    return state;
}

inline glm::vec2 widget_center(const glm::vec2& position,
                               const glm::vec2& size) {
    return position + (size / 2.f);
}

bool text(const uuid id, const WidgetConfig& config) {
    // NOTE: currently id is only used for focus and hot/active,
    // we could potentially also track "selections"
    // with a range so the user can highlight text
    // not needed for supermarket but could be in the future?
    (void)id;
    // No need to render if text is empty
    if (config.text.empty()) return false;

    UIContext::FontPhraseTexInfo texInfo = get()->generateFontPhraseTexture(
        config.font, to_wstring(config.text), config.temporary);
    if (!texInfo.valid()) {
        log_error("failed to fetch texture for text {} with font {}",
                  config.text, config.font);
        return false;
    }
    auto font_scale = glm::vec2{texInfo.width, texInfo.height};
    font_scale /= (1.f * texInfo.fontSize);
    font_scale.y *= config.flipTextY ? -1.f : 1.f;

    auto size = config.size * font_scale;

    get()->drawWidget(widget_center(config.position, size),   //
                      config.size * font_scale,               //
                      config.rotation,                        //
                      config.theme.color(WidgetTheme::FONT),  //
                      texInfo.textureName);
    return true;
}

inline void activeIfMouseInside(const uuid id, const glm::vec4& rect) {
    bool inside = get()->isMouseInside(rect);
    if (inside) {
        get()->hotID = id;
        if (get()->activeID == rootID && get()->lmouseDown) {
            get()->activeID = id;
        }
    }
    return;
}

inline void _button_render(const uuid id, const WidgetConfig& config) {
    draw_if_kb_focus(id, [&]() {
        get()->drawWidget(config.position, config.size + glm::vec2{0.1f},
                          config.rotation, teal, config.theme.texture);
    });

#if 0
    if (get()->hotID == id) {
        if (get()->activeID == id) {
            get()->drawWidget(config.position, config.size, config.rotation,
                              red, config.theme.texture);
        } else {
            // Hovered
            get()->drawWidget(config.position, config.size, config.rotation,
                              green, config.theme.texture);
        }
    } else {
        get()->drawWidget(config.position, config.size, config.rotation, blue,
                          config.theme.texture);
    }
#endif
    get()->drawWidget(config.position, config.size, config.rotation,
                      config.theme.color(), config.theme.texture);

    if (config.text.size() != 0) {
        float sign = config.flipTextY ? 1 : -1;

        WidgetConfig textConfig(config);
        // TODO detect if the button color is dark
        // and change the color to white automatically
        textConfig.theme.fontColor = getOppositeColor(config.theme.color());
        textConfig.position =
            config.position + glm::vec2{(-config.size.x / 2.f) + 0.10f,
                                        config.size.y * sign * 0.5f};
        textConfig.size = 0.75f * glm::vec2{config.size.y, config.size.y};
        text(MK_UUID(id.ownerLayer, 0), textConfig);
    }
}

inline void handle_tabbing(const uuid id) {
    if (has_kb_focus(id)) {
        if (get()->pressed(get()->keyMapping["Widget Next"])) {
            get()->kbFocusID = rootID;
            if (get()->isKeyPressed(get()->keyMapping["Widget Mod"])) {
                get()->kbFocusID = get()->lastProcessed;
            }
        }
    }
    // before any returns
    get()->lastProcessed = id;
}

inline bool _button_pressed(const uuid id) {
    // check click
    if (has_kb_focus(id)) {
        if (get()->pressed(get()->keyMapping["Widget Press"])) {
            return true;
        }
    }
    if (!get()->lmouseDown && isActiveAndHot(id)) {
        get()->kbFocusID = id;
        return true;
    }
    return false;
}

bool button(const uuid id, WidgetConfig config) {
    // no state
    activeIfMouseInside(id, glm::vec4{config.position, config.size});
    config.position = widget_center(config.position, config.size);
    try_to_grab_kb(id);
    _button_render(id, config);
    handle_tabbing(id);
    bool pressed = _button_pressed(id);
    return pressed;
}

bool button_with_label(const uuid id, WidgetConfig config) {
    auto pressed = button(id, config);
    if (config.text == "") {
        // apply offset so text is relative to button position
        config.child->position += config.position;
        text(MK_UUID(id.ownerLayer, 0), *config.child);
    }
    return pressed;
}

bool button_list(const uuid id, WidgetConfig config,
                 const std::vector<WidgetConfig>& configs,
                 int* selectedIndex = nullptr, bool* hasFocus = nullptr) {
    auto state = widget_init<ButtonListState>(id);
    if (selectedIndex) state->selected.set(*selectedIndex);

    // TODO :HASFOCUS do we ever want to read the value
    // or do we want to reset focus each frame
    // if (hasFocus) state->hasFocus.set(*hasFocus);
    state->hasFocus.set(false);

    auto pressed = false;
    float spacing = config.size.y * 1.0f;
    float sign = config.flipTextY ? 1.f : -1.f;

    // Generate all the button ids
    std::vector<uuid> ids;
    for (size_t i = 0; i < configs.size(); i++) {
        ids.push_back(MK_UUID_LOOP(id.ownerLayer, id.hash, i));
    }

    for (size_t i = 0; i < configs.size(); i++) {
        const uuid button_id = ids[i];
        WidgetConfig bwlconfig(config);
        bwlconfig.position =
            config.position + glm::vec2{0.f, sign * spacing * (i + 1)};
        bwlconfig.text = configs[i].text;
        // bwlconfig.theme.backgroundColor = configs[i].theme.color();

        if (button_with_label(button_id, bwlconfig)) {
            state->selected = i;
            pressed = true;
        }
    }

    // NOTE: hasFocus is generally a readonly variable because
    // we dont insert it into state on button_list start see :HASFOCUS
    //
    // We use it here because we know that if its true, the caller
    // is trying to force this to have focus OR we had focus
    // last frame.
    //
    bool somethingFocused = hasFocus ? *hasFocus : false;
    // NOTE: this exists because we only to be able to move
    // up and down if we are messing with the button list directly
    // It would be annoying to be focused on the textfield (e.g.)
    // and pressing up accidentally would unfocus that and bring you to some
    // random button list somewhere**
    //
    // ** in this situation they have to be visible, so no worries about
    // arrow keying your way into a dropdown
    //
    for (size_t i = 0; i < configs.size(); i++) {
        somethingFocused |= has_kb_focus(ids[i]);
    }

    if (somethingFocused) {
        if (get()->pressed(get()->keyMapping["Value Up"])) {
            state->selected = state->selected - 1;
            if (state->selected < 0) state->selected = 0;
            get()->kbFocusID = ids[state->selected];
        }

        if (get()->pressed(get()->keyMapping["Value Down"])) {
            state->selected = state->selected + 1;
            if (state->selected > (int)configs.size() - 1)
                state->selected = configs.size() - 1;
            get()->kbFocusID = ids[state->selected];
        }
    }

    // NOTE: This has to be after value changes so that
    // hasFocus is handed back up to its parent correctly
    // this allows the dropdown to know when none of its children have focus
    // --
    // doing this above the keypress, allows a single frame
    // where neither the dropdown parent nor its children are in focus
    // and that causes the dropdown to close on selection change which isnt
    // what we want
    for (size_t i = 0; i < configs.size(); i++) {
        // if you got the kb focus somehow
        // (ie by clicking or tabbing)
        // then we will just make you selected
        if (has_kb_focus(ids[i])) state->selected = i;
        state->hasFocus = state->hasFocus | has_kb_focus(ids[i]);
    }

    if (state->hasFocus) get()->kbFocusID = ids[state->selected];
    if (hasFocus) *hasFocus = state->hasFocus;
    if (selectedIndex) *selectedIndex = state->selected;
    return pressed;
}

bool dropdown(const uuid id, WidgetConfig config,
              const std::vector<WidgetConfig>& configs, bool* dropdownState,
              int* selectedIndex) {
    auto state = widget_init<DropdownState>(id);
    if (dropdownState) state->on.set(*dropdownState);

    config.text = configs[selectedIndex ? *selectedIndex : 0].text;

    // Draw the main body of the dropdown
    // pressed
    auto pressed = button(id, config);

    // TODO when you tab to the dropdown
    // it would be nice if it opened
    // try_to_grab_kb(id);
    // if (has_kb_focus(id)) {
    // state->on = true;
    // }

    // TODO right now you can change values through tab or through
    // arrow keys, maybe we should only allow arrows
    // and then tab just switches to the next non dropdown widget

    // Text drawn after button so it shows up on top...
    //
    // TODO rotation is not really working correctly and so we have to
    // offset the V a little more than ^ in order to make it look nice
    auto offset = glm::vec2{config.size.x - (state->on ? 1.f : 1.6f),
                            config.size.y * -0.25f};
    text(MK_UUID(id.ownerLayer, id.hash),
         WidgetConfig({.theme = WidgetTheme(
                           {.fontColor = getOppositeColor(config.theme.color()),
                            .backgroundColor = config.theme.color()}),
                       .rotation = state->on ? 90.f : 270.f,
                       .text = ">",
                       .flipTextY = config.flipTextY,
                       .position = config.position + offset}));

    bool childrenHaveFocus = false;

    // NOTE: originally we only did this when the dropdown wasnt already
    // open but we should be safe to always do this
    // 1. doesnt toggle on, sets on directly
    // 2. open should have children focused anyway
    // 3. we dont eat the input, so it doesnt break the button_list value
    // up/down
    if (has_kb_focus(id)) {
        if (get()->pressedWithoutEat(get()->keyMapping["Value Up"]) ||
            get()->pressedWithoutEat(get()->keyMapping["Value Down"])) {
            state->on = true;
            childrenHaveFocus = true;
        }
    }

    if (state->on) {
        if (button_list(MK_UUID(id.ownerLayer, id.hash), config, configs,
                        selectedIndex, &childrenHaveFocus)) {
            state->on = false;
            get()->kbFocusID = id;
        }
    }

    auto ret =
        *dropdownState != state->on.asT() || (pressed && state->on.asT());

    // NOTE: this has to happen after ret
    // because its not a real selection, just
    // a tab out
    if (!childrenHaveFocus && !has_kb_focus(id)) {
        state->on = false;
    }

    if (pressed) state->on = !state->on;
    if (dropdownState) *dropdownState = state->on;
    return ret;
}

bool checkbox(const uuid id, WidgetConfig config, bool* cbState = nullptr) {
    auto state = widget_init<CheckboxState>(id);
    if (cbState) state->checked.set(*cbState);

    bool changed = false;
    auto textConf = WidgetConfig({
        .theme = WidgetTheme({
            .fontColor = glm::vec4{0.f, 0.f, 0.f, 1.f},
        }),
        .position = glm::vec2{0.1f, -0.25f},
        .text = state->checked ? "X" : "",
    });
    auto conf = WidgetConfig({
        .child = &textConf,           //
        .position = config.position,  //
        .size = config.size,          //
    });
    if (button_with_label(MK_UUID(id.ownerLayer, id.hash), conf)) {
        state->checked = !state->checked;
        changed = true;
    }

    // If the user specified an output
    if (cbState) (*cbState) = state->checked;
    return changed;
}

inline void _slider_render(const uuid id, const WidgetConfig& config,
                           const float value) {
    const auto cs = config.size;
    const float pos_offset = value * (config.vertical ? cs.y : cs.x);
    const auto col = isActiveOrHot(id) ? green : blue;
    const auto pos = widget_center(config.position, config.size);
    draw_if_kb_focus(id, [&]() {
        get()->drawWidget(pos,  //
                                // TODO should this be times because
                                // we want it to be the same % size increase
                                // no matter the size of the widget
                          config.size + glm::vec2{0.1f},  //
                          config.rotation, teal, config.theme.texture);
    });
    // slider rail
    get()->drawWidget(pos, cs, config.rotation, red, config.theme.texture);
    // slide
    glm::vec2 offset = config.vertical ? glm::vec2{cs.x / 2.f, pos_offset}
                                       : glm::vec2{pos_offset, cs.y / 2.f};
    get()->drawWidget(config.position + offset, glm::vec2{0.5f},
                      config.rotation, col, config.theme.texture);
}

bool slider(const uuid id, WidgetConfig config, float* value, float mnf,
            float mxf) {
    // TODO be able to scroll this bar with the scroll wheel
    auto state = widget_init<SliderState>(id);
    if (value) state->value.set(*value);

    activeIfMouseInside(id, glm::vec4{config.position, config.size});
    // dont mind if i do
    try_to_grab_kb(id);
    _slider_render(id, config, state->value.asT());
    handle_tabbing(id);

    bool value_changed = false;
    if (has_kb_focus(id)) {
        if (get()->isKeyPressed(get()->keyMapping["Value Up"])) {
            state->value = state->value + 0.005;
            if (state->value > mxf) state->value = mxf;

            (*value) = state->value;
            value_changed = true;
        }
        if (get()->isKeyPressed(get()->keyMapping["Value Down"])) {
            state->value = state->value - 0.005;
            if (state->value < mnf) state->value = mnf;
            (*value) = state->value;
            value_changed = true;
        }
    }

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
            value_changed = true;
        }
    }
    return value_changed;
}

// TODO add support for max-length textfield
// this will also help with temporary texture size

inline void _textfield_render(const uuid& id, const WidgetConfig& config,
                              const std::shared_ptr<TextfieldState>& state) {
    // Draw focus ring
    draw_if_kb_focus(id, [&]() {
        get()->drawWidget(config.position, config.size + glm::vec2{0.1f},
                          config.rotation, teal, config.theme.texture);
    });

    get()->drawWidget(config.position, config.size, config.rotation,
                      config.theme.color(), config.theme.texture);

    float tSize = config.size.y * 0.5f;
    auto tStartLocation =
        config.position - glm::vec2{config.size.x / 2.f,
                                    // only move text if not flipped otherwise
                                    // text will be too high in the box
                                    config.flipTextY ? -tSize : 0.5f};

    state->cursorBlinkTime = state->cursorBlinkTime + 1;
    if (state->cursorBlinkTime > 60) {
        state->cursorBlinkTime = 0;
        state->showCursor = !state->showCursor;
    }

    bool shouldWriteCursor = has_kb_focus(id) && state->showCursor;
    std::wstring focusStr = shouldWriteCursor ? L"_" : L"";
    std::wstring focused_content =
        fmt::format(L"{}{}", state->buffer.asT(), focusStr);

    text(MK_UUID(id.ownerLayer, id.hash),
         WidgetConfig({
             .theme = WidgetTheme({
                 .backgroundColor = glm::vec4{1.0, 0.8f, 0.5f, 1.0f},
             }),
             .position = tStartLocation,
             .size = glm::vec2{tSize},
             .text = to_string(focused_content),
             .flipTextY = config.flipTextY,
             .temporary = true,
         }));
}

bool textfield(const uuid id, WidgetConfig config, std::wstring& content) {
    auto state = widget_init<TextfieldState>(id);
    activeIfMouseInside(id, glm::vec4{config.position, config.size});
    // everything is drawn from the center so move it so its not the center
    // that way the mouse collision works
    config.position = widget_center(config.position, config.size);
    // if no one else has keyboard focus
    // dont mind if i do
    try_to_grab_kb(id);

    _textfield_render(id, config, state);

    bool changed = false;
    handle_tabbing(id);

    if (has_kb_focus(id)) {
        if (get()->keychar != int()) {
            state->buffer.asT().append(std::wstring(1, get()->keychar));
            changed = true;
        }
        if (get()->modchar == get()->keyMapping["Text Backspace"]) {
            if (state->buffer.asT().size() > 0) {
                state->buffer.asT().pop_back();
            }
            changed = true;
        }
    }

    if (!get()->lmouseDown && isActiveAndHot(id)) {
        get()->kbFocusID = id;
    }

    content = state->buffer;
    return changed;
}

typedef std::function<std::vector<std::string>(std::string)> AutoCompleteFn;

void _commandfield_autocmp_render(
    const uuid& id, const WidgetConfig& config,
    const std::shared_ptr<CommandfieldState>& state) {
    if (state->autocomp.asT().size() && state->selected >= -1) {
        int si = state->selected;
        std::vector<WidgetConfig> autocompConfigs;
        glm::vec4 hoverColor = green;
        for (int i = 0; i < (int)state->autocomp.asT().size(); i++) {
            autocompConfigs.push_back(WidgetConfig(
                {.text = state->autocomp.asT()[i],
                 .theme = WidgetTheme(
                     {.backgroundColor =
                          i == si ? hoverColor : config.theme.color()})}));
        }

        bool childrenHaveFocus = true;
        if (button_list(MK_UUID(id.ownerLayer, id.hash), config,
                        autocompConfigs, &si, &childrenHaveFocus)) {
            state->buffer.asT() = to_wstring(state->autocomp.asT().at(si));
            state->autocomp.asT().clear();
        }
        if (si != state->selected) {
            state->buffer.asT().clear();
        }
        state->selected = si;
    }
}

inline void _commandfield_process_enter(
    const std::shared_ptr<CommandfieldState>& state, std::wstring& content) {
    if (state->selected > 0) {
        const auto ac = state->autocomp.asT().at(state->selected);
        state->buffer.asT() = to_wstring(ac);
    }
    content = state->buffer.asT();
    state->autocomp.asT().clear();
    state->selected = -1;
    state->buffer.asT().clear();
}

bool commandfield(const uuid id, WidgetConfig config, std::wstring& content,
                  AutoCompleteFn generateAutoComplete = AutoCompleteFn()) {
    //
    // TODO can we bold the letters that match in the trie?
    // TODO add support for selecting suggestion with keyboard
    //
    auto state = widget_init<CommandfieldState>(id);

    // log_info("index: {} autocmp size {} buffer {}",
    // state->selected.asT(), state->autocomp.asT().size(),
    // to_string(state->buffer));

    // NOTE: We dont need a separate ID since we want
    // global state to match and for them to have the
    // same keyboard focus
    auto changed = textfield(id, config, content);

    bool commandRun = false;

    if (has_kb_focus(id)) {
        if (get()->pressed(get()->keyMapping["Command Enter"])) {
            _commandfield_process_enter(state, content);
            commandRun = true;
        }

        // if autocomplete is empty or we recently typed something
        // regerate the autocomplete and selected the first item
        if (state->autocomp.asT().empty() || changed) {
            state->autocomp = generateAutoComplete(to_string(state->buffer));
            state->selected = -1;
        }
    } else {
        // This ends up being kinda clear-on-exit in a way
        // but also for clear-on-tab (this hides the button_list)
        state->autocomp.asT().clear();
        commandRun = false;
    }
    // TODO - should we bring back clearing the command line?

    _commandfield_autocmp_render(id, config, state);
    handle_tabbing(id);
    return commandRun;
}

bool drawer(const uuid id, WidgetConfig config, float* pct_open = nullptr) {
    auto state = widget_init<DrawerState>(id);
    if (pct_open) state->heightPct = *pct_open;

    if (state->heightPct < 1.f) {
        state->heightPct.asT() += 0.1;
    }
    get()->drawWidget(
        glm::vec2{config.position.x, state->heightPct * config.position.y},
        config.size, config.rotation, config.theme.color(),
        config.theme.texture);
    // output pct if user wants
    if (pct_open) *pct_open = state->heightPct;
    // return if drawer should render children
    if (state->heightPct > 0.9) return true;
    return false;
}

bool window(const uuid&, const WidgetConfig& config) {
    get()->drawWidget(config.position, config.size, config.rotation,
                      config.theme.color(), config.theme.texture);
    return true;
}

bool scroll_view(const uuid id, WidgetConfig config,
                 std::vector<Child> children, float itemHeight,
                 int* startingIndex = nullptr) {
    // TODO is there a way for us to render all of this to a separate
    // texture? then we can just hide part of it on the gpu based on y
    // position or something

    // TODO smooth scrolling

    auto state = widget_init<ScrollViewState>(id);
    if (startingIndex) state->yoffset = (*startingIndex) * itemHeight;

    activeIfMouseInside(id, glm::vec4{config.position, config.size});

    // everything is drawn from the center so move it so its not the center
    // that way the mouse collision works
    config.position = widget_center(config.position, config.size);

    // TODO add scrollbar
    int itemsInFrame = ceil(config.size.y / itemHeight);
    int numItems = children.size() - itemsInFrame;

    if (get()->hotID == id) {
        // mouse over our window
        float newoff =                    //
            (config.flipTextY ? -1 : -1)  //
            * (config.size.y * 0.5f)      //
            * get()->yscrolled;
        state->yoffset = state->yoffset + newoff;
        state->yoffset = fmin(state->yoffset, numItems * itemHeight);
        state->yoffset = fmax(state->yoffset, 0);
        get()->yscrolled = 0.f;
    }

    if (!config.transparent) {
        get()->drawWidget(config.position, config.size, config.rotation,
                          config.theme.color(), config.theme.texture);
    }
    int startIndex = ceil(state->yoffset / itemHeight);
    int endIndex = startIndex + itemsInFrame;

    float dir = config.flipTextY ? -1 : 1;
    float startPos = config.position.y;
    if (config.flipTextY) startPos -= 3.5f * itemHeight;
    startPos += (dir * itemHeight);  // align with top of box

    for (int i = startIndex; i < endIndex; i++) {
        if (i < 0) continue;
        if (i >= (int)children.size()) break;

        float ypos =  //
            startPos  // global alignment with box
            - (dir * itemHeight * (i - startIndex))  // move into box
            ;

        children[i](WidgetConfig({
            .position =
                glm::vec2{config.position.x - config.size.x / 2.f, ypos},
            .size = glm::vec2{0.f, itemHeight},
        }));
    }

    if (startingIndex) *startingIndex = ceil(state->yoffset / itemHeight);
    return false;
}

// TODO not ready for use, need to mess around with the location / scales
// some more
inline bool plusMinusButton(const uuid id, WidgetConfig config,
                            float* value = nullptr, float increment = 0.1) {
    auto state = widget_init<PlusMinusButtonState>(id);
    if (value) state->value = *value;
    bool changed = false;

    WidgetConfig original(config);

    config.size /= 2.f;
    config.position += glm::vec2{0, +config.size.y};
    config.text = ">";
    config.rotation = 270.f;
    if (button(MK_UUID(id.ownerLayer, id.hash), config)) {
        state->value = state->value + increment;
        changed = true;
    }

    config.rotation = 90.f;
    config.position.y -= config.size.y * 2;
    if (button(MK_UUID(id.ownerLayer, id.hash), config)) {
        state->value = state->value + increment;
        changed = true;
    }

    original.position.x += config.size.x;
    text(MK_UUID(id.ownerLayer, id.hash), original);

    // TODO allow support for setting these
    state->value = fmaxf(0, state->value);
    state->value = fminf(9999, state->value);

    if (value) *value = state->value;
    return changed;
}

}  // namespace GOUI
