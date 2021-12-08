
#pragma once
#include "app.h"
#include "event.h"
#include "graphicscontext.h"
#include "input.h"
#include "keycodes.h"
#include "log.h"
#include "pch.hpp"
#include "window.h"

struct OpenGLWindow : public Window {
    GLFWwindow* window;
    GraphicsContext* context;

    struct WindowInfo {
        std::string title;
        int width, height;
        bool vsync;
        EventCallbackFn callback;
    };
    WindowInfo info;

    OpenGLWindow(const WindowConfig& config) { init(config); }
    virtual ~OpenGLWindow() {
        delete context;
        close();
    }
    inline virtual int width() const override { return info.width; }
    inline virtual int height() const override { return info.height; }
    inline virtual bool isVSync() const override { return info.vsync; }
    inline virtual void* getNativeWindow() const override { return window; }

    virtual void setEventCallback(const EventCallbackFn& callback) override {
        info.callback = callback;
    }

    virtual void update() override {
        prof(__PROFILE_FUNC__);
        glfwPollEvents();
        context->swapBuffers();
    }

    virtual void setVSync(bool enabled) override {
        if (enabled) {
            glfwSwapInterval(1);
        } else {
            glfwSwapInterval(0);
        }
        info.vsync = enabled;
    }

    virtual void init(const WindowConfig& config);
    virtual void close() { glfwTerminate(); }
};

