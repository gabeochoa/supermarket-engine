
#pragma once

#include <functional>

#include "layer.hpp"
#include "log.h"
#include "pch.hpp"
#include "shader.h"
#include "window.h"

#define M_BIND(x) std::bind(&App::x, this, std::placeholders::_1)

struct App {
    std::unique_ptr<Window> window;
    std::unique_ptr<Shader> shader;
    bool running;
    LayerStack layerstack;

    unsigned int vertexArray, vertexBuffer, indexBuffer, shaderProgram;

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

        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        float vertices[9] = {
            -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f,
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
                     GL_STATIC_DRAW);

        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        int indices[3] = {0, 1, 2};
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                     GL_STATIC_DRAW);

        std::string vertex_shader = R"(
            #version 400
            in vec3 vp;

            out vec3 op;

            void main(){
            op = vp;
                gl_Position = vec4(vp, 1.0);
            }
        )";

        std::string fragment_shader = R"(
            #version 400
            in vec3 op;
            out vec4 frag_color;

            void main(){
                frag_color = vec4(op*0.5 + 0.5, 1.0);
            }
        )";

        shader.reset(new Shader(vertex_shader, fragment_shader));
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
        }
        return true;
    }

    void onEvent(Event& e) {
        log_trace(e.toString());
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
            shader->bind();
            glBindVertexArray(vertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);

            for (Layer* layer : layerstack) {
                layer->onUpdate();
            }

            window->update();
        }
        return 0;
    }
};

