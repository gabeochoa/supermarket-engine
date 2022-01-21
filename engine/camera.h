#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "event.h"
#include "input.h"
#include "pch.hpp"

struct Camera {};

struct OrthoCamera : public Camera {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 viewProjection;
    glm::vec3 position;
    glm::vec4 viewport;
    // TODO maybe switch to quaternion later?
    float rotation;

    OrthoCamera(float left, float right, float bottom, float top)
        : projection(glm::ortho(left, right, bottom, top, -1.f, 1.f)),

          view(1.0f),
          position(0.0f),
          rotation(0.0f) {
        updateViewMat();
    }

    void setViewport(const glm::vec4& vp);
    void setProjection(float left, float right, float bottom, float top);
    void setPosition(glm::vec3 newpos);
    void updateViewMat();
};

struct OrthoCameraController {
    float aspectRatio;
    float zoomLevel;
    float camSpeed;
    float rotSpeed;
    bool movementEnabled;
    bool rotationEnabled;
    bool zoomEnabled = true;
    bool resizeEnabled = true;
    OrthoCamera camera;

    OrthoCameraController(float ratio, float defaultZoom = 1.f,
                          float cameraSpeed = 5.f, float rotationSpeed = 180.f)
        : aspectRatio(ratio),
          zoomLevel(defaultZoom),
          camSpeed(cameraSpeed),
          rotSpeed(rotationSpeed),
          movementEnabled(cameraSpeed != 0.f),
          rotationEnabled(rotationSpeed != 0.f),
          camera(-ratio * zoomLevel, ratio * zoomLevel, -zoomLevel, zoomLevel) {
        setZoomLevel(defaultZoom);
    }

    void onUpdate(Time dt);
    void setZoomLevel(float zm);
    bool onMouseScrolled(Mouse::MouseScrolledEvent& event);
    bool onWindowResized(WindowResizeEvent& event);
    void onEvent(Event& event);
};
