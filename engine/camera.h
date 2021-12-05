#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "pch.hpp"

struct Camera {};

struct OrthoCamera : public Camera {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 viewProjection;
    glm::vec3 position;
    // TODO maybe switch to quaternion later?
    float rotation;

    OrthoCamera(float left, float right, float bottom, float top)
        : projection(glm::ortho(left, right, bottom, top, -1.f, 1.f)),

          view(1.0f) {
        viewProjection = projection * view;
    }

    void updateViewMat() {
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), position) *
            glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));

        view = glm::inverse(transform);
        viewProjection = projection * view;
    }
};
