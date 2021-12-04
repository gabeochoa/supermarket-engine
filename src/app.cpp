
#include "pch.hpp"
#include "ogl.h"

struct App {
    App() {}
    int run() {
        GLFWwindow* window = init_opengl();
        if (window == nullptr) {
            std::cout << "Failed to grab window" << std::endl;
            return -1;
        }

        double last = glfwGetTime();
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window)) {
            double now = glfwGetTime();
            double elapsed = now - last;
            last = now;

            /* Render here */
            glClear(GL_COLOR_BUFFER_BIT |
                    GL_DEPTH_BUFFER_BIT);  // Clear the buffers

            glDrawArrays(GL_TRIANGLES, 0, 3);

            tick(elapsed);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

        glfwTerminate();
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
