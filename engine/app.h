
#pragma once

#include <functional>

#include "buffer.h"
#include "layer.hpp"
#include "log.h"
#include "pch.hpp"
#include "shader.h"
#include "window.h"

#define M_BIND(x) std::bind(&App::x, this, std::placeholders::_1)

struct App {
    std::unique_ptr<Window> window;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> shader2;

    std::shared_ptr<VertexArray> vertexArray;

    std::shared_ptr<VertexArray> squareVA;

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

        {
            vertexArray.reset(VertexArray::create());
            std::shared_ptr<VertexBuffer> vertexBuffer;
            std::shared_ptr<IndexBuffer> indexBuffer;

            float vertices[] = {
                -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                0.5f,  -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                0.0f,  0.5f,  0.0f, 1.f,  0.f,  1.f,  1.f,
            };
            vertexBuffer.reset(
                VertexBuffer::create(vertices, sizeof(vertices)));

            BufferLayout layout = {
                {"vp", BufferType::Float3},
                {"color", BufferType::Float4},
            };
            vertexBuffer->setLayout(layout);
            vertexArray->addVertexBuffer(vertexBuffer);

            unsigned int indices[3] = {0, 1, 2};
            indexBuffer.reset(IndexBuffer::create(indices, 3));
            vertexArray->setIndexBuffer(indexBuffer);

            std::string vertex_shader = R"(
            #version 400
            in vec3 vp;
            in vec4 color;

            out vec3 op;
            out vec4 oc;

            void main(){
                op = vp;
                gl_Position = vec4(vp, 1.0);
                oc = color;
            }
        )";

            std::string fragment_shader = R"(
            #version 400
            in vec3 op;
            in vec4 oc;

            out vec4 frag_color;

            void main(){
                frag_color = vec4(op*0.5 + 0.5, 1.0);
                frag_color = oc;
            }
        )";

            shader.reset(new Shader(vertex_shader, fragment_shader));
        }
        {
            std::shared_ptr<VertexBuffer> squareVB;
            std::shared_ptr<IndexBuffer> squareIB;
            squareVA.reset(VertexArray::create());

            float squareVerts[] = {
                -0.5f, -0.5f, 0.0f,  // one
                0.5f,  -0.5f, 0.0f,  // two
                0.5f,  0.5f,  0.f,   //
                -0.5f, 0.0f,  0.f,   //
            };
            squareVB.reset(
                VertexBuffer::create(squareVerts, sizeof(squareVerts)));
            squareVB->setLayout(BufferLayout{
                {"vp", BufferType::Float3},
            });
            squareVA->addVertexBuffer(squareVB);

            unsigned int squareIs[] = {0, 1, 2, 2, 3, 0};
            squareIB.reset(IndexBuffer::create(
                squareIs, sizeof(squareIs) / sizeof(unsigned int)));
            squareVA->setIndexBuffer(squareIB);

            std::string vertex_shader2 = R"(
            #version 400
            in vec3 vp;

            out vec3 op;

            void main(){
                op = vp;
                gl_Position = vec4(vp, 1.0);
            }
        )";

            std::string fragment_shader2 = R"(
            #version 400
            in vec3 op;

            out vec4 frag_color;

            void main(){
                frag_color = vec4(op*0.5 + 0.5, 1.0);
            }
        )";

            shader2.reset(new Shader(vertex_shader2, fragment_shader2));
        }
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

            shader2->bind();
            squareVA->bind();
            glDrawArrays(GL_TRIANGLES, 0, squareVA->indexBuffer->getCount());

            shader->bind();
            vertexArray->bind();
            glDrawArrays(GL_TRIANGLES, 0, vertexArray->indexBuffer->getCount());

            for (Layer* layer : layerstack) {
                layer->onUpdate();
            }

            window->update();
        }
        return 0;
    }
};

