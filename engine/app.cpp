
#include "app.h"

static std::shared_ptr<App> app;

App& App::get() { return *app; }

App::App(AppSettings settings) {
    this->settings = settings;
    isMinimized = false;
    running = true;
    GLOBALS.set<bool>("__engine__app_running", &running);

    WindowConfig config;
    config.width = settings.width;
    config.height = settings.height;
    config.title = settings.title;

    window = std::unique_ptr<Window>(Window::create(config));

    M_ASSERT(window, "failed to grab window");

    Key::load_keys();

    window->setEventCallback(M_BIND(onEvent));

    Renderer::init();
}

App::~App() { Key::export_keys(); }

bool App::onWindowClose(WindowCloseEvent& event) {
    (void)event;
    running = false;
    return true;
}

bool App::onKeyPressed(KeyPressedEvent& event) {
    if (event.keycode == Key::mapping["Esc"] && settings.escClosesWindow) {
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
    return false;
}

void App::onEvent(Event& e) {
    log_trace(e.toString().c_str());
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
