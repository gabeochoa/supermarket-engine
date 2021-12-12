
#pragma once

#include "pch.hpp"

// https://www.khronos.org/opengl/wiki/Shader_Compilation
struct Shader {
    std::string name;
    int rendererID;

    Shader(const std::string &n, const std::string &vertexSource,
           const std::string &fragmentSource)
        : name(n) {
        std::unordered_map<GLenum, std::string> shaderSources;
        shaderSources[GL_VERTEX_SHADER] = vertexSource;
        shaderSources[GL_FRAGMENT_SHADER] = fragmentSource;
        compile(shaderSources);
    }

    Shader(const std::string &filepath) {
        name = nameFromFilePath(filepath);

        log_trace(fmt::format("Trying to load shader at: {} ", filepath));
        std::string contents = readFromFile(filepath);
        log_trace(fmt::format("Got shader contents from file {} ", filepath));
        std::unordered_map<GLenum, std::string> sources = preProcess(contents);
        log_trace(fmt::format("finished preprocessing shader {} ", filepath));
        compile(sources);
    }

    std::unordered_map<GLenum, std::string> preProcess(
        const std::string &source) {
        std::unordered_map<GLenum, std::string> shaderSources;

        const char *typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        // Start of shader type declaration line
        size_t pos = source.find(typeToken, 0);
        while (pos != std::string::npos) {
            // End of shader type declaration line
            size_t eol = source.find_first_of("\r\n", pos);
            M_ASSERT(eol != std::string::npos, "Syntax error in shader source");
            size_t begin =
                pos + typeTokenLength +
                1;  // Start of shader type name (after "#type " keyword)
            std::string type = source.substr(begin, eol - begin);
            M_ASSERT(typeFromString(type), "Invalid shader type specified");

            // Start of shader code after shader type declaration line
            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            M_ASSERT(nextLinePos != std::string::npos,
                     "Syntax error in shader source");
            // Start of next shader type declaration line
            pos = source.find(typeToken, nextLinePos);

            shaderSources[typeFromString(type)] =
                (pos == std::string::npos)
                    ? source.substr(nextLinePos)
                    : source.substr(nextLinePos, pos - nextLinePos);
        }
        return shaderSources;
    }

    GLenum typeFromString(const std::string &type) {
        if (type == "vertex") return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;

        M_ASSERT(false, fmt::format("Unknown shader type: {}", type));
        return 0;
    }

    std::string readFromFile(const std::string &filepath) {
        std::string result;
        std::ifstream in(
            filepath,
            std::ios::in |
                std::ios::binary);  // ifstream closes itself due to RAII

        if (!in.is_open()) {
            log_warn(
                fmt::format("Tried to load shader: {} but failed", filepath));
            return result;
        }

        in.seekg(0, std::ios::end);
        auto size = in.tellg();
        if (size != -1) {
            result.resize(size);
            in.seekg(0, std::ios::beg);
            in.read(&result[0], size);
        }
        return result;
    }

    void compile(const std::unordered_map<GLenum, std::string> &shaderSources) {
        auto program = glCreateProgram();
        std::vector<GLenum> shaderIDs;
        shaderIDs.reserve(shaderSources.size());

        for (auto &kv : shaderSources) {
            GLenum type = kv.first;
            const std::string &source = kv.second;
            // Create an empty vertex shader handle
            GLuint shader = glCreateShader(type);

            // Send the vertex shader source code to GL
            // Note that std::string's .c_str is NULL character terminated.
            auto source_cstr = source.c_str();
            glShaderSource(shader, 1, &source_cstr, 0);

            // Compile the vertex shader
            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE) {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                // The maxLength includes the NULL character
                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                // We don't need the shader anymore.
                glDeleteShader(shader);

                // Use the infoLog as you see fit.
                log_error(
                    fmt::format("{} Shader failed to compile: \n{}", type,
                                std::string(infoLog.begin(), infoLog.end())));
                return;
            }

            glAttachShader(program, shader);
            shaderIDs.push_back(shader);
        }

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
            for (auto id : shaderIDs) {
                glDeleteShader(id);
            }

            // Use the infoLog as you see fit.

            // In this simple program, we'll just leave
            log_error(fmt::format("Shaders failed to link:\n {}",
                                  std::string(infoLog.begin(), infoLog.end())));
            return;
        }

        // Always detach shaders after a successful link.
        for (auto id : shaderIDs) {
            glDetachShader(program, id);
        }

        rendererID = program;
    }

    ~Shader() { glDeleteProgram(rendererID); }

    void bind() const { glUseProgram(rendererID); }
    void unbind() const { glUseProgram(0); }
    void uploadUniformInt(const std::string &name, const int i) {
        GLint location = glGetUniformLocation(rendererID, name.c_str());
        glUniform1i(location, i);
    }
    void uploadUniformIntArray(const std::string &name, int* values, int count) {
        GLint location = glGetUniformLocation(rendererID, name.c_str());
        glUniform1iv(location, count, values);
    }
    void uploadUniformFloat(const std::string &name, float value) {
        GLint location = glGetUniformLocation(rendererID, name.c_str());
        glUniform1f(location, value);
    }
    void uploadUniformFloat3(const std::string &name, const glm::vec3 &values) {
        GLint location = glGetUniformLocation(rendererID, name.c_str());
        glUniform3f(location, values.x, values.y, values.z);
    }
    void uploadUniformFloat4(const std::string &name, const glm::vec4 &values) {
        GLint location = glGetUniformLocation(rendererID, name.c_str());
        glUniform4f(location, values.x, values.y, values.z, values.w);
    }
    void uploadUniformMat4(const std::string &name, const glm::mat4 &matrix) {
        GLint location = glGetUniformLocation(rendererID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }
};

struct ShaderLibrary {
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;

    void add(const std::shared_ptr<Shader> &shader) {
        if (shaders.find(shader->name) != shaders.end()) {
            log_warn(
                fmt::format("Failed to add shader to library, shader with name "
                            "{} already exists",
                            shader->name));
            return;
        }
        log_trace(
            fmt::format("Adding Shader \"{}\" to our library", shader->name));
        shaders[shader->name] = shader;
    }
    std::shared_ptr<Shader> load(const std::string &path) {
        auto shader = std::make_shared<Shader>(path);
        add(shader);
        return shader;
    }

    std::shared_ptr<Shader> get(const std::string &name) {
        return shaders[name];
    }
};
