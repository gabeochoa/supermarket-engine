
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
#include "shader.h"
#include "time.h"
#include "window.h"

#define M_BIND(x) std::bind(&App::x, this, std::placeholders::_1)

struct AppSettings {
    int width;
    int height;
    const char* title;
    // should the app manager clear before drawing layers?
    bool clearEnabled = false;
    // should this close the window when user hits escape?
    bool escClosesWindow = false;
};

struct App {
    std::unique_ptr<Window> window;
    Time time;
    bool isMinimized;
    AppSettings settings;

    bool running;
    LayerStack layerstack;

    static void create(AppSettings settings) ;
    static App& get();

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


[[deprecated("You should use App::get()")]] 
static std::shared_ptr<App> app__DO_NOT_USE;

