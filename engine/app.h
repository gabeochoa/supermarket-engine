
#pragma once

#include <functional>

#include "buffer.h"
#include "camera.h"
#include "edit.h"
#include "keycodes.h"
#include "layer.h"
#include "log.h"
#include "pch.hpp"
#include "renderer.h"
#include "resources.h"
#include "shader.h"
#include "time.h"
#include "window.h"

#define M_BIND(x) std::bind(&App::x, this, std::placeholders::_1)

struct AppSettings {
    int width;
    int height;
    float ratio = 0.f;
    const char* title;
    // should the app manager clear before drawing layers?
    bool clearEnabled = false;
    // should this close the window when user hits escape?
    bool escClosesWindow = false;
    // if not empty, init resources folder to this
    std::string initResourcesFolder = "";
};

struct App {
    std::unique_ptr<Window> window;
    Time time;
    bool isMinimized;
    AppSettings settings;

    bool running;
    LayerStack layerstack;

    static void create(AppSettings settings);
    static App& get();
    static AppSettings& getSettings();

    App(AppSettings settings);
    ~App();

    bool onWindowClose(WindowCloseEvent& event);
    bool onKeyPressed(KeyPressedEvent& event);
    bool onWindowResized(WindowResizeEvent& event);
    void onEvent(Event& e);
    void pushLayer(Layer* layer);
    void pushOverlay(Layer* layer);
    Window& getWindow();
    int run();
};

[[deprecated("You should use App::get()")]] static std::shared_ptr<App>
    app__DO_NOT_USE;

