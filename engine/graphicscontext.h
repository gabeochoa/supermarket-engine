
#pragma once
#include "pch.hpp"

struct GLFWwindow;

struct GraphicsContext {
    virtual ~GraphicsContext() {}
    virtual void init() = 0;
    virtual void swapBuffers() = 0;
};

struct OpenGLContext : public GraphicsContext {
    GLFWwindow* handle;
    OpenGLContext(GLFWwindow* h) : handle(h) {}
    virtual void init() override { glfwMakeContextCurrent(handle); }
    virtual void swapBuffers() override { glfwSwapBuffers(handle); }
};
