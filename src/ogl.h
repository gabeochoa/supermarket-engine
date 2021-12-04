
#pragma once 

#include "pch.hpp"


#define BUFFER_OFFSET(i) ((char*)NULL + (i))

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


GLFWwindow* init_opengl(){
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) {
        return nullptr;
    }

#ifdef __APPLE__
    /* We need to explicitly ask for a 3.2 context on OS X */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return nullptr;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    auto err = glewInit();
    if (err != GLEW_OK) {
        std::cout << "failed to init glew: " << err << std::endl;
        return nullptr;
    }

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    float positions[6] = {
        -0.5f, -0.5f, 0.0f, 0.5f, 0.5f, -0.5f,
    };
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
    std::string vertexShader =
        "#version 410 core\n"
        "layout(location = 0) in vec4 position;\n"
        "void main(){\n"
        "   gl_Position = position;\n"
        "}\n";

    std::string fragmentShader =
        "#version 410 core\n"
        "layout(location = 0) out vec4 color;\n"
        "void main(){\n"
        "   color = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    unsigned int shader =
        Program::Load(vertexShader.c_str(), GL_VERTEX_SHADER,
                      fragmentShader.c_str(), GL_FRAGMENT_SHADER, NULL);
    glUseProgram(shader);

    return window;
}
