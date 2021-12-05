
#pragma once

#include <glm/gtc/type_ptr.hpp>

#include "pch.hpp"

////// ////// ////// ////// ////// ////// ////// //////
//              Flat Color Shader
////// ////// ////// ////// ////// ////// ////// //////

static std::string flat_vert_s = R"(
    #version 400
    in vec3 i_pos;
    uniform mat4 viewProjection;
    uniform mat4 transformMatrix;

    out vec3 o_pos;

    void main(){
        o_pos = i_pos;
        gl_Position = viewProjection * transformMatrix * vec4(i_pos, 1.0);
    }
)";

static std::string flat_frag_s = R"(
    #version 400
    in vec3 position;
    uniform vec4 u_color;

    out vec4 frag_color;
    void main(){
        frag_color = vec4(0.3, 0.8, 0.3, 1.0);
        frag_color = u_color;
    }
)";

////// ////// ////// ////// ////// ////// ////// //////

// https://www.khronos.org/opengl/wiki/Shader_Compilation
struct Shader {
    int rendererID;

    Shader(const std::string &vertexSource, const std::string &fragmentSource) {
        // Create an empty vertex shader handle
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

        // Send the vertex shader source code to GL
        // Note that std::string's .c_str is NULL character terminated.
        auto source = vertexSource.c_str();
        glShaderSource(vertexShader, 1, &source, 0);

        // Compile the vertex shader
        glCompileShader(vertexShader);

        GLint isCompiled = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(vertexShader, maxLength, &maxLength,
                               &infoLog[0]);

            // We don't need the shader anymore.
            glDeleteShader(vertexShader);

            // Use the infoLog as you see fit.
            log_error(fmt::format("Vertex Shader failed to compile: \n{}",
                                  std::string(infoLog.begin(), infoLog.end())));
            return;
        }

        // Create an empty fragment shader handle
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        // Send the fragment shader source code to GL
        // Note that std::string's .c_str is NULL character terminated.
        source = (const GLchar *)fragmentSource.c_str();
        glShaderSource(fragmentShader, 1, &source, 0);

        // Compile the fragment shader
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(fragmentShader, maxLength, &maxLength,
                               &infoLog[0]);

            // We don't need the shader anymore.
            glDeleteShader(fragmentShader);
            // Either of them. Don't leak shaders.
            glDeleteShader(vertexShader);

            // Use the infoLog as you see fit.
            log_error(fmt::format("Fragment Shader failed to compile:\n {}",
                                  std::string(infoLog.begin(), infoLog.end())));
            return;
        }

        // Vertex and fragment shaders are successfully compiled.
        // Now time to link them together into a program.
        // Get a program object.
        rendererID = glCreateProgram();
        auto program = rendererID;

        // Attach our shaders to our program
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        // Link our program
        glLinkProgram(program);

        // Note the different functions here: glGetProgram* instead of
        // glGetShader*.
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            // We don't need the program anymore.
            glDeleteProgram(program);
            // Don't leak shaders either.
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            // Use the infoLog as you see fit.

            // In this simple program, we'll just leave
            log_error(fmt::format("Shader failed to link:\n {}",
                                  std::string(infoLog.begin(), infoLog.end())));
            return;
        }

        // Always detach shaders after a successful link.
        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
    }

    ~Shader() { glDeleteProgram(rendererID); }

    void bind() const { glUseProgram(rendererID); }
    void unbind() const { glUseProgram(0); }
    void uploadUniformFloat4(const std::string &name, const glm::vec4 &values) {
        GLint location = glGetUniformLocation(rendererID, name.c_str());
        glUniform4f(location, values.x, values.y, values.z, values.w);
    }
    void uploadUniformMat4(const std::string &name, const glm::mat4 &matrix) {
        GLint location = glGetUniformLocation(rendererID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }
};
