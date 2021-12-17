#pragma once

#include "../engine/pch.hpp"

inline glm::vec2 lerp(const glm::vec2& x, const glm::vec2& y, float t) {
    return x * (1.f - t) + y * t;
}

inline glm::vec2 gen_rand_vec2(float min, float max) {
    return {
        glm::linearRand(min, max),
        glm::linearRand(min, max),
    };
}

inline glm::vec3 gen_rand_vec3(float min, float max) {
    return {
        glm::linearRand(min, max),
        glm::linearRand(min, max),
        glm::linearRand(min, max),
    };
}

inline glm::vec4 gen_rand_vec4(float min, float max) {
    return {
        glm::linearRand(min, max),
        glm::linearRand(min, max),
        glm::linearRand(min, max),
        glm::linearRand(min, max),
    };
}

struct VectorHash {
    std::size_t operator()(const glm::vec2& a) const {
        return std::hash<float>()(a.x) ^ ((std::hash<float>()(a.y) << 1) >> 1);
    }
};

inline glm::vec3 worldToScreen(const glm::vec3& position, const glm::mat4& view,
                               const glm::mat4& projection,
                               const glm::vec4& viewport) {
    return glm::project(position, view, projection, viewport);
}

inline glm::vec3 screenToWorld(const glm::vec3& position, const glm::mat4& view,
                               const glm::mat4& projection,
                               const glm::vec4& viewport) {
    return glm::unProject(position, view, projection, viewport);
}
