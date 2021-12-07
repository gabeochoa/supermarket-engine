
#pragma once

#include <functional>

#include "buffer.h"
#include "camera.h"
#include "layer.h"
#include "log.h"
#include "pch.hpp"
#include "renderer.h"
#include "shader.h"
#include "time.h"
#include "window.h"

#define M_BIND(x) std::bind(&App::x, this, std::placeholders::_1)

struct App {
    std::unique_ptr<Window> window;
    Time time;
    bool isMinimized;

    bool running;
    LayerStack layerstack;

    inline static App& get() {
        static App app;
        return app;
    }

    App() {
        WindowConfig config;
        config.width = 1920;
        config.height = 1080;
        config.title = "test tile";

        window = std::unique_ptr<Window>(Window::create(config));

        M_ASSERT(window, "failed to grab window");

        Key::load_keys();

        running = true;
        window->setEventCallback(M_BIND(onEvent));

        Renderer::init();
    }

    ~App() { Key::export_keys(); }

    bool onWindowClose(WindowCloseEvent& event) {
        (void)event;
        running = false;
        return true;
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        // TODO should i be creating a window close event instead of just
        // stopping run?
        if (event.keycode == Key::mapping["Esc"]) {
            running = false;
            return true;
        }
        return false;
    }

    bool onWindowResized(WindowResizeEvent& event) {
        if (event.width() == 0 || event.height() == 0) {
            isMinimized = true;
        }
        isMinimized = false;
        Renderer::resize(event.width(), event.height());
        return false;
    }

    void onEvent(Event& e) {
        log_trace(e.toString());
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(M_BIND(onWindowClose));
        dispatcher.dispatch<KeyPressedEvent>(M_BIND(onKeyPressed));
        dispatcher.dispatch<WindowResizeEvent>(M_BIND(onWindowResized));
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

    void pushLayer(Layer* layer) { layerstack.push(layer); }
    void pushOverlay(Layer* layer) { layerstack.pushOverlay(layer); }

    Window& getWindow() { return *window; }

    int run() {
        time.start();
        while (running) {
            time.end();

            if (isMinimized) {
                continue;
            }

            for (Layer* layer : layerstack) {
                layer->onUpdate(time);
            }
            window->update();
        }
        return 0;
    }
};

