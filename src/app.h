
#pragma once

#include <functional>

#include "layer.hpp"
#include "log.h"
#include "pch.hpp"
#include "window.h"

#define M_BIND(x) std::bind(&App::x, this, std::placeholders::_1)

struct App {
    std::unique_ptr<Window> window;
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
        running = true;
        window->setEventCallback(M_BIND(onEvent));

        Key::load_keys();
    }

    ~App() {
        // TODO export keys,
        // i have it off since the folder doesnt exist anyway
        // Key::export_keys();
    }

    bool onWindowClose(WindowCloseEvent& event) {
        running = false;
        return true;
    }

    bool onKeyPressed(KeyPressedEvent& event) {
        // TODO should i be creating a window close event instead of just
        // stopping run?
        if (event.keycode == Key::mapping["Esc"]) {
            running = false;
        }
        return true;
    }

    void onEvent(Event& e) {
        log(e.toString());
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(M_BIND(onWindowClose));
        dispatcher.dispatch<KeyPressedEvent>(M_BIND(onKeyPressed));

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
        while (running) {
            glClear(GL_COLOR_BUFFER_BIT |
                    GL_DEPTH_BUFFER_BIT);  // Clear the buffers

            for (Layer* layer : layerstack) {
                layer->onUpdate();
            }

            window->update();
        }
        return 0;
    }
};

