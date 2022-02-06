#pragma once
#include "external_include.h"
//

#include "event.h"
#include "input.h"
#include "pch.hpp"

struct Camera {
    glm::mat4 projection;
    Camera() = default;
    Camera(const glm::mat4& projection, const glm::mat4 view,
           const glm::vec3 position)
        : projection(projection), view(view), position(position) {}

    glm::mat4 view;
    glm::vec3 position;
};

struct OrthoCamera : public Camera {
    glm::mat4 viewProjection;
    glm::vec4 viewport;
    // TODO maybe switch to quaternion later?
    float rotation;

    OrthoCamera(float left, float right, float bottom, float top)
        : Camera(glm::ortho(left, right, bottom, top, -1.f, 1.f),
                 glm::mat4{1.f}, glm::vec3{0.f}),
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

struct FreeCamera : public Camera {
    float pitch = 0.0f, yaw = 0.0f;
    glm::vec3 focalPoint = {0.0f, 0.0f, 0.0f};

    float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f,
          m_FarClip = 10000.0f;

    glm::vec2 m_InitialMousePosition = {0.0f, 0.0f};
    float distance = 10.0f;
    float m_ViewportWidth = 1280, m_ViewportHeight = 720;

    FreeCamera() = default;
    FreeCamera(float fov, float aspectRatio, float nearClip, float farClip);

    void onUpdate(Time dt);
    void onEvent(Event& e);

    inline void setViewport(const glm::vec4& vp) {
        m_ViewportWidth = vp.z;
        m_ViewportHeight = vp.w;
        UpdateProjection();
    }

    inline void setPosition(const glm::vec3& p) {
        position = p;
        UpdateView();
    }

    glm::mat4 getViewProjection() const { return projection * view; }

    void UpdateProjection();
    void UpdateView();

    bool onMouseScroll(Mouse::MouseScrolledEvent& e);
    bool onKeyPressed(KeyPressedEvent& e);

    void pan(const glm::vec2& delta);
    void MouseRotate(const glm::vec2& delta);
    void setZoomLevel(float delta);

    glm::vec2 getPanSpeed() const;
    float RotationSpeed() const;
    float getZoomSpeed() const;

    glm::vec3 getUpDirection() const;
    glm::vec3 getRightDirection() const;
    glm::vec3 getForwardDirection() const;
    glm::vec3 calculatePosition() const;
    glm::quat getOrientation() const;
};
