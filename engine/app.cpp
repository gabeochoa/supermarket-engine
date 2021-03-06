
#include "app.h"

#include "edit.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

App& App::get() { return *GLOBALS.get_ptr<App>("app"); }
AppSettings& App::getSettings() {
    return *GLOBALS.get_ptr<AppSettings>("__engine__app_settings");
}

void App::create(AppSettings settings) {
    App* app = new App(settings);
    app__DO_NOT_USE.reset(app);
    GLOBALS.set<App>("app", app);
    GLOBALS.set<AppSettings>("__engine__app_settings",
                             &app__DO_NOT_USE->settings);
}

App::App(AppSettings settings) {
    this->settings = settings;
    isMinimized = false;
    running = true;
    GLOBALS.set<bool>("__engine__app_running", &running);

    this->settings.ratio = 1.f * settings.width / settings.height;

    WindowConfig config;
    config.width = settings.width;
    config.height = settings.height;
    config.title = settings.title;

    window = std::unique_ptr<Window>(Window::create(config));

    Key::initMapping();

    M_ASSERT(window, "failed to grab window");

    window->setEventCallback(M_BIND(onEvent));

    if (!settings.initResourcesFolder.empty()) {
        ResourceLocations& resources = getResourceLocations();
        resources.folder = settings.initResourcesFolder;
        resources.init();
    }

    Renderer::init();
}

App::~App() {}

bool App::onWindowClose(WindowCloseEvent& event) {
    (void)event;
    running = false;
    return true;
}

bool App::onKeyPressed(KeyPressedEvent& event) {
    if (event.keycode == Key::getMapping("Esc") && settings.escClosesWindow) {
        running = false;
        return true;
    }
    return false;
}

bool App::onWindowResized(WindowResizeEvent& event) {
    if (event.width() == 0 || event.height() == 0) {
        isMinimized = true;
    }
    isMinimized = false;
    Renderer::resize(event.width(), event.height());
    this->settings.width = event.width();
    this->settings.height = event.height();
    this->settings.ratio = 1.f * this->settings.width / this->settings.height;
    return false;
}

void App::onEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<WindowCloseEvent>(M_BIND(onWindowClose));
    dispatcher.dispatch<KeyPressedEvent>(M_BIND(onKeyPressed));
    dispatcher.dispatch<WindowResizeEvent>(M_BIND(onWindowResized));
    if (e.handled) {
        return;
    }

    // Have the top most layers get the event first,
    // if they handle it then no need for the lower ones to get the rest
    // eg imagine UI pause menu blocking game UI elements
    //    we wouldnt want the player to click pass the pause menu
    for (auto it = layerstack.end(); it != layerstack.begin();) {
        (*--it)->onEvent(e);
        if (e.handled) {
            break;
        }
    }
}

void App::pushLayer(Layer* layer) { layerstack.push(layer); }
void App::pushOverlay(Layer* layer) { layerstack.pushOverlay(layer); }

Window& App::getWindow() { return *window; }

int App::run() {
    time.start();
    while (running) {
        prof give_me_a_name(__PROFILE_FUNC__);
        time.end();
        Renderer::stats.reset();
        Renderer::stats.begin();
        if (isMinimized) continue;
        if (settings.clearEnabled)
            Renderer::clear(/* color */ {0.1f, 0.1f, 0.1f, 1.0f});
        for (Layer* layer : layerstack) {
            layer->onUpdate(time);
        }
        window->update();
        Renderer::stats.end();
    }
    return 0;
}

#pragma clang diagnostic pop
