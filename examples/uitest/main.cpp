
//
#include "../../engine/ui.h"
#include "../../engine/uihelper.h"
//

#include "../../engine/app.h"
#include "../../engine/camera.h"
#include "../../engine/commands.h"
#include "../../engine/edit.h"
#include "../../engine/font.h"
#include "../../engine/layer.h"
#include "../../engine/pch.hpp"
#include "../../engine/terminal_layer.h"

constexpr int WIN_W = 1920;
constexpr int WIN_H = 1080;
constexpr float WIN_RATIO = WIN_W * 1.f / WIN_H;

struct UITestLayer : public Layer {
    float value = 0.08f;
    std::wstring content = L"";
    const int TYPABLE_START = 32;
    const int TYPABLE_END = 126;
    bool checkbox_state = false;
    int dropdownIndex = 0;
    bool dropdownState = false;
    int buttonListIndex = 0;
    float upperCaseRotation = 0.f;
    bool camHasMovement = false;
    bool camHasRotation = false;
    bool camHasZoom = false;
    std::wstring commandContent;

    std::shared_ptr<IUI::UIContext> ui_context;
    std::shared_ptr<OrthoCameraController> uiTestCameraController;

    UITestLayer() : Layer("UI Test") {
        uiTestCameraController.reset(new OrthoCameraController(
            WIN_RATIO, 10.f /*default zoom*/, 5.f /*cam speed*/,
            90.f /*rotation speed*/));
        uiTestCameraController->camera.setPosition(glm::vec3{15.f, 0.f, 0.f});
        camHasMovement = uiTestCameraController->movementEnabled;
        camHasRotation = uiTestCameraController->rotationEnabled;
        camHasZoom = uiTestCameraController->zoomEnabled;

        uiTestCameraController->camera.setViewport(
            glm::vec4{0, 0, WIN_W, WIN_H});

        ui_context.reset(new IUI::UIContext());
        IUI::init_uicontext(ui_context.get(), uiTestCameraController);
        ui_context->c_id = 1;

        GLOBALS.set<float>("slider_val", &value);

        // We register some command with similar names
        // to test the auto complete feature
        EDITOR_COMMANDS.registerCommand("zzzz_1", ToggleBoolCommand<bool>(),
                                        "example zzz");
        EDITOR_COMMANDS.registerCommand("zzzz_2", ToggleBoolCommand<bool>(),
                                        "example zzz");
        EDITOR_COMMANDS.registerCommand("zzzz_3", ToggleBoolCommand<bool>(),
                                        "example zzz");
    }

    virtual ~UITestLayer() {}
    virtual void onAttach() override {}
    virtual void onDetach() override {}

    virtual void onUpdate(Time dt) override {
        log_trace("{:.2}s ({:.2} ms) ", dt.s(), dt.ms());

        if (GLOBALS.get_or_default<bool>("terminal_closed", true)) {
            uiTestCameraController->onUpdate(dt);
        }

        Renderer::begin(uiTestCameraController->camera);
        ui_test(dt);
        Renderer::end();
    }

    void ui_test(Time dt) {
        using namespace IUI;
        auto mouseDown =
            Input::isMouseButtonPressed(Mouse::MouseCode::ButtonLeft);
        auto mousePosition =
            screenToWorld(glm::vec3{Input::getMousePosition(), 0.f},
                          uiTestCameraController->camera.view,        //
                          uiTestCameraController->camera.projection,  //
                          uiTestCameraController->camera.viewport     //
            );

        ui_context->begin(mouseDown, mousePosition);

        if (button(MK_UUID(id, rootID),
                   WidgetConfig({.position = glm::vec2{0.f, 0.f},
                                 .size = glm::vec2{2.f, 1.f}}))) {
            log_info("clicked button");
        }

        if (button(MK_UUID(id, rootID),
                   WidgetConfig({.position = glm::vec2{3.f, -2.f},
                                 .size = glm::vec2{6.f, 1.f},
                                 .text = "open file dialog"}))) {
            auto files = getFilesFromUser();
            for (auto file : files) log_info("You chose the file: {}", file);
        }
        if (button(MK_UUID(id, rootID),
                   WidgetConfig({.position = glm::vec2{2.5f, 0.f},
                                 .size = glm::vec2{6.f, 1.f},
                                 .text = "open mesage box"}))) {
            auto result =
                openMessage("Example", "This is an example message box");
            auto str = (result == 0 ? "cancel" : (result == 1 ? "yes" : "no"));
            log_info("they clicked: {}", str);
        }

        if (slider(MK_UUID(id, rootID),
                   WidgetConfig({
                       .position = glm::vec2{9.f, 0.f},
                       .size = glm::vec2{1.f, 3.f},
                       .vertical = true,
                   }),
                   GLOBALS.get_ptr<float>("slider_val"), 0.08f, 0.95f)) {
            // log_info("idk moved slider? ");
        }

        if (slider(MK_UUID(id, rootID),
                   WidgetConfig({
                       .position = glm::vec2{11.f, 3.f},
                       .size = glm::vec2{3.f, 1.f},
                       .vertical = false,
                   }),
                   GLOBALS.get_ptr<float>("slider_val"), 0.08f, 0.95f)) {
            // log_info("idk moved slider? ");
        }

        upperCaseRotation += 90.f * dt.s();
        auto upperCaseConfig =
            WidgetConfig({.color = glm::vec4{1.0, 0.8f, 0.5f, 1.0f},
                          .position = glm::vec2{0.f, -6.f},
                          .rotation = upperCaseRotation,
                          .size = glm::vec2{1.f, 1.f},
                          .text = "THE FIVE BOXING WIZARDS JUMP QUICKLY"});

        auto lowerCaseConfig =
            WidgetConfig({.color = glm::vec4{0.8, 0.3f, 0.7f, 1.0f},
                          .position = glm::vec2{0.f, -8.f},
                          .size = glm::vec2{1.f, 1.f},
                          .text = "the five boxing wizards jump quickly"});

        auto numbersConfig =
            WidgetConfig({.color = glm::vec4{0.7, 0.5f, 0.8f, 1.0f},
                          .position = glm::vec2{0.f, -10.f},
                          .size = glm::vec2{1.f, 1.f},
                          .text = "0123456789"});

        auto extrasConfig =
            WidgetConfig({.color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
                          .position = glm::vec2{0.f, -12.f},
                          .size = glm::vec2{1.f, 1.f},
                          .text = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"});

        auto hiraganaConfig = WidgetConfig({
            .color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
            .font = "default_cjk",
            .position = glm::vec2{0.f, -14.f},
            .size = glm::vec2{1.f, 1.f},
            .text = "Hiragana: "
                    "\xe3\x81\x8b\xe3\x81\x8d\xe3\x81\x8f\xe3"
                    "\x81\x91\xe3\x81\x93",
        });
        auto kanjiConfig = WidgetConfig({
            .color = glm::vec4{0.2, 0.7f, 0.4f, 1.0f},
            .font = "default_cjk",
            .position = glm::vec2{0.f, -16.f},
            .size = glm::vec2{1.f, 1.f},
            .text = "Kanji: \xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e",
        });

        text(MK_UUID(id, IUI::rootID), upperCaseConfig);
        text(MK_UUID(id, IUI::rootID), lowerCaseConfig);
        text(MK_UUID(id, IUI::rootID), numbersConfig);
        text(MK_UUID(id, IUI::rootID), extrasConfig);
        text(MK_UUID(id, IUI::rootID), hiraganaConfig);
        text(MK_UUID(id, IUI::rootID), kanjiConfig);

        uuid textFieldID = MK_UUID(id, IUI::rootID);
        if (textfield(textFieldID,
                      WidgetConfig({.position = glm::vec2{2.f, 2.f},
                                    .size = glm::vec2{6.f, 1.f}}),
                      content)) {
            log_info("{}", to_string(content));
        }

        uuid commandFieldID = MK_UUID(id, IUI::rootID);

        auto genAutoComplete = [](const std::string& input) {
            return EDITOR_COMMANDS.tabComplete(input);
        };

        if (commandfield(commandFieldID,
                         WidgetConfig({.position = glm::vec2{2.f, 4.f},
                                       .size = glm::vec2{6.f, 1.f}}),
                         commandContent, genAutoComplete)) {
            log_info("running command {}", to_string(commandContent));
            EDITOR_COMMANDS.triggerCommand(to_string(commandContent));
            commandContent = L"";
            if (!EDITOR_COMMANDS.command_history.empty()) {
                log_info("Just ran a command ... {}",
                         EDITOR_COMMANDS.command_history.back());
            }
        }

        // In this case we want to lock the camera when typing in
        // this specific textfield
        // TODO should this be the default?
        // TODO should this live in the textFieldConfig?
        uiTestCameraController->movementEnabled = camHasMovement;
        uiTestCameraController->rotationEnabled = camHasRotation;
        if (uiTestCameraController->movementEnabled &&
            (IUI::has_kb_focus(textFieldID) ||
             IUI::has_kb_focus(commandFieldID))) {
            uiTestCameraController->movementEnabled = false;
            uiTestCameraController->rotationEnabled = false;
        }

        auto tapToContiueText = WidgetConfig({
            .position = glm::vec2{0.5f, 0.f},
            .size = glm::vec2{0.75f, 0.75f},
            .text = "Tap to continue",
        });

        if (IUI::button_with_label(
                IUI::MK_UUID(id, IUI::rootID),
                IUI::WidgetConfig({
                    .child = &tapToContiueText,                 //
                    .color = glm::vec4{0.3f, 0.9f, 0.5f, 1.f},  //
                    .position = glm::vec2{12.f, 1.f},           //
                    .size = glm::vec2{8.f, 1.f},                //
                    .transparent = false,                       //
                })                                              //
                )) {
        }

        if (IUI::checkbox(                      //
                IUI::MK_UUID(id, IUI::rootID),  //
                IUI::WidgetConfig({
                    .color = glm::vec4{0.6f, 0.3f, 0.3f, 1.f},  //
                    .position = glm::vec2{16.f, 3.f},           //
                    .size = glm::vec2{1.f, 1.f},                //
                    .transparent = false,                       //
                }),                                             //
                &checkbox_state)) {
            log_info("checkbox changed {}", checkbox_state);
        }

        {
            std::vector<WidgetConfig> dropdownConfigs;
            dropdownConfigs.push_back(IUI::WidgetConfig({.text = "option A"}));
            dropdownConfigs.push_back(IUI::WidgetConfig({.text = "option B"}));
            dropdownConfigs.push_back(
                IUI::WidgetConfig({.text = "long option"}));
            dropdownConfigs.push_back(
                IUI::WidgetConfig({.text = "really really long option"}));

            WidgetConfig dropdownMain = IUI::WidgetConfig({
                .color = glm::vec4{0.3f, 0.9f, 0.5f, 1.f},  //
                .position = glm::vec2{12.f, -1.f},          //
                .size = glm::vec2{8.5f, 1.f},               //
                .text = "",                                 //
                .transparent = false,                       //
            });

            if (IUI::dropdown(IUI::MK_UUID(id, IUI::rootID), dropdownMain,
                              dropdownConfigs, &dropdownState,
                              &dropdownIndex)) {
                log_info("dropdown selected {}",
                         dropdownConfigs[dropdownIndex].text);
            }
        }

        {
            //

            std::vector<WidgetConfig> buttonListConfigs;
            buttonListConfigs.push_back(IUI::WidgetConfig({.text = "button1"}));
            buttonListConfigs.push_back(IUI::WidgetConfig({.text = "button2"}));
            buttonListConfigs.push_back(IUI::WidgetConfig({.text = "button3"}));

            WidgetConfig buttonListConfig = IUI::WidgetConfig({
                .color = glm::vec4{0.3f, 0.9f, 0.5f, 1.f},  //
                .position = glm::vec2{22.f, 6.f},           //
                .size = glm::vec2{8.5f, 1.f},               //
                .text = "",                                 //
                .transparent = false,                       //
            });

            if (IUI::button_list(IUI::MK_UUID(id, IUI::rootID),
                                 buttonListConfig, buttonListConfigs,
                                 &buttonListIndex)) {
                log_info("button in list {} pressed", buttonListIndex);
            }
        }

        const uuid scroll_view_id = IUI::MK_UUID(id, IUI::rootID);
        {
            WidgetConfig scrollViewConfig = IUI::WidgetConfig({
                .color = glm::vec4{0.3f, 0.9f, 0.5f, 1.f},  //
                .position = glm::vec2{22.f, -3.f},          //
                .size = glm::vec2{8.5f, 4.f},               //
                .text = "",                                 //
                .transparent = false,                       //
            });

            std::vector<IUI::Child> rows;
            for (int i = 0; i < 10; i++) {
                rows.push_back([i](WidgetConfig config) {
                    config.size.x = 1.f;
                    config.text = fmt::format("row{}", i),
                    IUI::text(
                        // text doesnt need ids really
                        IUI::MK_UUID_LOOP(0, 0, i), config);
                });
            }

            IUI::scroll_view(scroll_view_id, scrollViewConfig, rows, 1.f);
        }

        // In this case we want to lock the camera when typing in
        // this specific textfield
        // TODO should this be the default?
        // TODO should this live in the textFieldConfig?
        uiTestCameraController->zoomEnabled = camHasZoom;
        if (uiTestCameraController->zoomEnabled &&
            ui_context->hotID == scroll_view_id) {
            uiTestCameraController->zoomEnabled = false;
        }

        ui_context->end();
    }

    bool onMouseButtonPressed(Mouse::MouseButtonPressedEvent& e) {
        (void)e;
        return false;
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        if (event.keycode == Key::getMapping("Esc")) {
            App::get().running = false;
            return true;
        }
        if (ui_context->processKeyPressEvent(event.keycode)) {
            return true;
        }
        return false;
    }

    virtual void onEvent(Event& event) override {
        log_warn(event.toString().c_str());

        if (!GLOBALS.get<bool>("terminal_closed")) {
            return;
        }

        uiTestCameraController->onEvent(event);
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<Mouse::MouseButtonPressedEvent>(std::bind(
            &UITestLayer::onMouseButtonPressed, this, std::placeholders::_1));
        dispatcher.dispatch<KeyPressedEvent>(
            std::bind(&UITestLayer::onKeyPressed, this, std::placeholders::_1));

        dispatcher.dispatch<CharPressedEvent>(
            [this](CharPressedEvent& event) -> bool {
                ui_context->processCharPressEvent(event.charcode);
                return true;
            });
        dispatcher.dispatch<Mouse::MouseScrolledEvent>(
            [this](Mouse::MouseScrolledEvent& event) -> bool {
                ui_context->processMouseScrolled(event.GetYOffset());
                return true;
            });
    }
};

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    ResourceLocations& resources = getResourceLocations();
    resources.folder = "../resources";
    resources.init();

    App::create({
        .width = WIN_W,
        .height = WIN_H,
        .title = "uitest",
        .clearEnabled = true,
        .escClosesWindow = false,
    });

    Layer* terminal = new TerminalLayer();
    App::get().pushLayer(terminal);

    Layer* uitest = new UITestLayer();
    App::get().pushLayer(uitest);

    App::get().run();

    return 0;
}
