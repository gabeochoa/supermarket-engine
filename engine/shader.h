
#pragma once

#include "file.h"
#include "pch.hpp"

// https://www.khronos.org/opengl/wiki/Shader_Compilation
struct Shader {
    std::string name;
    int rendererID;

    Shader(const std::string &n, const std::string &vertexSource,
           const std::string &fragmentSource);

    Shader(const std::string &filepath);
    Shader(const std::string &name, const char *data, int size);

    std::unordered_map<GLenum, std::string> preProcess(
        const std::string &source);

    GLenum typeFromString(const std::string &type);
    const char *typeToString(GLenum type);
    std::string readFromFile(const std::string &filepath);

    void compile(const std::unordered_map<GLenum, std::string> &shaderSources);

    ~Shader();

    void bind() const;
    void unbind() const;
    void uploadUniformInt(const std::string &fieldName, const int i);
    void uploadUniformIntArray(const std::string &fieldName, int *values,
                               int count);
    void uploadUniformFloat(const std::string &fieldName, float value);
    void uploadUniformFloat3(const std::string &fieldName,
                             const glm::vec3 &values);
    void uploadUniformFloat4(const std::string &fieldName,
                             const glm::vec4 &values);
    void uploadUniformMat4(const std::string &fieldName,
                           const glm::mat4 &matrix);
};

struct ShaderLibrary {
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    void add(const std::shared_ptr<Shader> &shader);
    std::shared_ptr<Shader> load(const std::string &path);
    std::shared_ptr<Shader> load_binary(const std::string &name,
                                        const char *data, int size);
    std::shared_ptr<Shader> get(const std::string &name);
};
