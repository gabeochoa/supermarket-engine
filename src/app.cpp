
#include <functional>

#include "pch.hpp"
#include "window.hpp"

#define M_BIND(x) std::bind(&App::x, this, std::placeholders::_1)
struct App {
    std::unique_ptr<Window> window;
    bool running;
    App() {
        WindowConfig config;
        config.width = 1920;
        config.height = 1080;
        config.title = "test tile";

        window = std::unique_ptr<Window>(Window::create(config));
        M_ASSERT(window, "failed to grab window");
        running = true;
        window->setEventCallback(M_BIND(onEvent));
    }

    bool onWindowClose(WindowCloseEvent& event) {
        running = false;
        return true;
    }

    void onEvent(Event& e) {
        log(e);
        EventDispatcher dispatcher(e);
        dispatcher.dispatch<WindowCloseEvent>(M_BIND(onWindowClose));
    }

    int run() {
        while (running) {
            glClear(GL_COLOR_BUFFER_BIT |
                    GL_DEPTH_BUFFER_BIT);  // Clear the buffers

            window->update();
        }
        return 0;
    }

    void tick(double elapsed) {}
};

int main(int argc, char** argv) {
    App* app = new App();

    app->run();

    delete app;

    return 0;
}
