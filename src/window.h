#pragma once

#include "event.hpp"
#include "pch.hpp"

typedef std::function<void(Event&)> EventCallbackFn;

struct WindowConfig {
    std::string title;
    int width = 0;
    int height = 0;
    bool vsync;
    EventCallbackFn callback;
};

struct Window {
    virtual ~Window() {}
    virtual void update() = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;

    virtual void setEventCallback(const EventCallbackFn& callback) = 0;
    virtual void setVSync(bool enabled) = 0;
    virtual bool isVSync() const = 0;
    inline virtual void* getNativeWindow() const = 0;
    static Window* create(const WindowConfig& config = WindowConfig());
};
