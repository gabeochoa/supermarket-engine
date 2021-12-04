#pragma once

#include "pch.hpp"
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace Mouse;

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct Program {
    static GLuint Load(const char* shader, ...) {
        GLuint prog = glCreateProgram();
        va_list args;
        va_start(args, shader);
        while (shader) {
            const GLenum type = va_arg(args, GLenum);
            AttachShader(prog, type, shader);
            shader = va_arg(args, const char*);
        }
        va_end(args);
        glLinkProgram(prog);
        CheckStatus(prog);
        return prog;
    }

   private:
    static void CheckStatus(GLuint obj) {
        GLint status = GL_FALSE;
        if (glIsShader(obj)) glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
        if (glIsProgram(obj)) glGetProgramiv(obj, GL_LINK_STATUS, &status);
        if (status == GL_TRUE) return;
        GLchar log[1 << 15] = {0};
        if (glIsShader(obj)) glGetShaderInfoLog(obj, sizeof(log), NULL, log);
        if (glIsProgram(obj)) glGetProgramInfoLog(obj, sizeof(log), NULL, log);
        std::cerr << log << std::endl;
        std::exit(EXIT_FAILURE);
    }

    static void AttachShader(GLuint program, GLenum type, const char* src) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &src, NULL);
        glCompileShader(shader);
        CheckStatus(shader);
        glAttachShader(program, shader);
        glDeleteShader(shader);
    }
};

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
    static Window* create(const WindowConfig& config = WindowConfig());
};

static bool wasOpenGLInitPreviously = false;
struct OpenGLWindow : public Window {
    GLFWwindow* window;
    struct WindowInfo {
        std::string title;
        int width, height;
        bool vsync;
        EventCallbackFn callback;
    };
    WindowInfo info;

    OpenGLWindow(const WindowConfig& config) { init(config); }
    virtual ~OpenGLWindow() { close(); }
    virtual int width() const override { return info.width; }
    virtual int height() const override { return info.height; }
    virtual bool isVSync() const override { return info.vsync; }

    virtual void setEventCallback(const EventCallbackFn& callback) override {
        info.callback = callback;
    }

    virtual void update() override {
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    virtual void setVSync(bool enabled) override {
        if (enabled) {
            glfwSwapInterval(1);
        } else {
            glfwSwapInterval(0);
        }
        info.vsync = enabled;
    }

    virtual void init(const WindowConfig& config) {
        info.title = config.title;
        info.width = config.width;
        info.height = config.height;

        log(fmt::format("Creating a new window {} ({}, {})", info.title,
                        info.width, info.height));

        if (!wasOpenGLInitPreviously) {
            wasOpenGLInitPreviously = true;

            /* Initialize the library */
            int success = glfwInit();
            M_ASSERT(success, "Failed to init glfw");
        }

#ifdef __APPLE__
        /* We need to explicitly ask for a 3.2 context on OS X */
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
        window = glfwCreateWindow(info.width, info.height, info.title.c_str(),
                                  NULL, NULL);
        M_ASSERT(window, "Failed to create wwindow");
        glfwMakeContextCurrent(window);

        glfwSetWindowUserPointer(window, &info);
        setVSync(true);
        auto err = glewInit();
        M_ASSERT(err == GLEW_OK, "Failed to init glew");

        glfwSetWindowSizeCallback(
            window, [](GLFWwindow* w, int width, int height) {
                WindowInfo& info = *(WindowInfo*)glfwGetWindowUserPointer(w);
                info.width = width;
                info.height = height;

                WindowResizeEvent event(width, height);
                info.callback(event);
            });

        glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
            WindowInfo& info = *(WindowInfo*)glfwGetWindowUserPointer(w);
            WindowCloseEvent event;
            info.callback(event);
        });

        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode,
                                      int action, int mods) {
            WindowInfo& info = *(WindowInfo*)glfwGetWindowUserPointer(w);
            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(key, 0);
                    info.callback(event);
                } break;
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(key);
                    info.callback(event);
                } break;
                case GLFW_REPEAT: {
                    KeyPressedEvent event(key, 1);
                    info.callback(event);
                } break;
            }
        });

        glfwSetMouseButtonCallback(
            window, [](GLFWwindow* w, int button, int action, int mods) {
                WindowInfo& info = *(WindowInfo*)glfwGetWindowUserPointer(w);

                switch (action) {
                    case GLFW_PRESS: {
                        MouseButtonPressedEvent event(button);
                        info.callback(event);
                        break;
                    }
                    case GLFW_RELEASE: {
                        MouseButtonReleasedEvent event(button);
                        info.callback(event);
                        break;
                    }
                }
            });

        glfwSetScrollCallback(
            window, [](GLFWwindow* w, double xOffset, double yOffset) {
                WindowInfo& info = *(WindowInfo*)glfwGetWindowUserPointer(w);

                MouseScrolledEvent event((float)xOffset, (float)yOffset);
                info.callback(event);
            });

        glfwSetCursorPosCallback(
            window, [](GLFWwindow* w, double xPos, double yPos) {
                WindowInfo& info = *(WindowInfo*)glfwGetWindowUserPointer(w);

                MouseMovedEvent event((float)xPos, (float)yPos);
                info.callback(event);
            });
    }

    virtual void close() { glfwTerminate(); }
};

Window* Window::create(const WindowConfig& config) {
    return new OpenGLWindow(config);
}
