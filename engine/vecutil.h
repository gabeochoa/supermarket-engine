#pragma once

#include "external_include.h"

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

// TODO should this live in some Math.h header?
template <typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

// This rounds to the Larger number maintaining the sign
// for negative this means lower, for pos this means larger
template <typename T>
T round_higher(T val) {
    int sign = sgn<T>(val);
    if (sign < 0) {
        return floor(val);
    }
    return ceil(val);
}

template <>
inline glm::vec2 round_higher(glm::vec2 val) {
    return glm::vec2{round_higher(val.x), round_higher(val.y)};
}

