
#pragma once

#include "app.h"
#include "event.h"
#include "graphicscontext.h"
#include "keycodes.h"
#include "log.h"
#include "pch.hpp"
#include "window.h"

// Probably should be called SFML window but a little too late
struct OpenGLWindow : public Window {
    sf::Window* window;

    struct WindowInfo {
        std::string title;
        int width, height;
        bool vsync;
        EventCallbackFn callback;
    };
    WindowInfo info;

    OpenGLWindow(const WindowConfig& config) { init(config); }
    virtual ~OpenGLWindow() { close(); }
    inline virtual void setActive(bool active) override {
        window->setActive(active);
    }
    inline virtual int width() const override { return info.width; }
    inline virtual int height() const override { return info.height; }
    inline virtual bool isVSync() const override { return info.vsync; }
    inline virtual void* getNativeWindow() const override { return window; }

    virtual void setEventCallback(const EventCallbackFn& callback) override {
        info.callback = callback;
    }

    virtual void update() override;
    void pollForEvents();

    virtual void setVSync(bool enabled) override {
        M_ASSERT(window, "Trying to set VSync on uninitialized window");
        window->setVerticalSyncEnabled(enabled);
        info.vsync = enabled;
    }

    virtual void init(const WindowConfig& config);
    virtual void close() {}
};

