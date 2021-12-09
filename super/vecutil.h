#pragma once

#include "../engine/pch.hpp"

glm::vec2 gen_rand_vec2(float min, float max) {
    return {
        glm::linearRand(min, max),
        glm::linearRand(min, max),
    };
}

glm::vec3 gen_rand_vec3(float min, float max) {
    return {
        glm::linearRand(min, max),
        glm::linearRand(min, max),
        glm::linearRand(min, max),
    };
}

glm::vec4 gen_rand_vec4(float min, float max) {
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
