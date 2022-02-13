
#include "camera.h"

#include "event.h"

void OrthoCamera::setViewport(const glm::vec4& vp) { viewport = vp; }

void OrthoCamera::setProjection(float left, float right, float bottom,
                                float top) {
    prof give_me_a_name(__PROFILE_FUNC__);
    projection = glm::ortho(left, right, bottom, top, -1.f, 1.f);
    updateViewMat();
}

void OrthoCamera::setPosition(glm::vec3 newpos) {
    position = newpos;
    updateViewMat();
}

void OrthoCamera::updateViewMat() {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) *
                          glm::rotate(glm::mat4(1.0f), glm::radians(rotation),
                                      glm::vec3(0, 0, 1));

    view = glm::inverse(transform);
    viewProjection = projection * view;
}

void OrthoCameraController::onUpdate(Time dt) {
    prof give_me_a_name(__PROFILE_FUNC__);
    if (movementEnabled) {
        if (Input::isKeyPressed(Key::getMapping("Left"))) {
            this->camera.position.x -= camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::getMapping("Right"))) {
            this->camera.position.x += camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::getMapping("Down"))) {
            this->camera.position.y -= camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::getMapping("Up"))) {
            this->camera.position.y += camSpeed * dt;
        }
    }
    if (rotationEnabled) {
        if (Input::isKeyPressed(Key::getMapping("Rotate Clockwise"))) {
            this->camera.rotation += rotSpeed * dt;
        }
        if (Input::isKeyPressed(Key::getMapping("Rotate Counterclockwise"))) {
            this->camera.rotation -= rotSpeed * dt;
        }
    }
    this->camera.updateViewMat();
}

void OrthoCameraController::setZoomLevel(float zm) {
    if (!zoomEnabled) return;
    zoomLevel = zm;
    camera.setProjection(-aspectRatio * zoomLevel, aspectRatio * zoomLevel,
                         -zoomLevel, zoomLevel);
    camSpeed = zoomLevel;
}

bool OrthoCameraController::onMouseScrolled(Mouse::MouseScrolledEvent& event) {
    float zl = zoomLevel - (event.GetYOffset() * 0.25);
    // float zl = fmin(fmax(zoomLevel - (event.GetYOffset() * 0.25), 0.5f), 20.f);
    setZoomLevel(zl);
    // TODO should this be true since we dont want to scroll and zoom at the
    // same time
    return false;
}

bool OrthoCameraController::onWindowResized(WindowResizeEvent& event) {
    if (!resizeEnabled) {
        float newWidth = aspectRatio * event.height();
        camera.setProjection(0.f, newWidth, event.height(), 0.f);
        return false;
    }

    aspectRatio = (1.f * event.width()) / event.height();
    camera.setProjection(-aspectRatio * zoomLevel, aspectRatio * zoomLevel,
                         -zoomLevel, zoomLevel);
    return false;
}

void OrthoCameraController::onEvent(Event& event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<Mouse::MouseScrolledEvent>(std::bind(
        &OrthoCameraController::onMouseScrolled, this, std::placeholders::_1));
    dispatcher.dispatch<WindowResizeEvent>(std::bind(
        &OrthoCameraController::onWindowResized, this, std::placeholders::_1));
}

////////
////////
////////

////////
////////
////////

FreeCamera::FreeCamera(float fov, float aspectRatio, float nearClip,
                       float farClip)
    : Camera(
          glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip),
          glm::mat4{1.f}, glm::vec3{0}),
      m_FOV(fov),
      m_AspectRatio(aspectRatio),
      m_NearClip(nearClip),
      m_FarClip(farClip) {
    UpdateView();
}

void FreeCamera::UpdateProjection() {
    m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
    projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio,
                                  m_NearClip, m_FarClip);
}

void FreeCamera::UpdateView() {
    position = calculatePosition();
    glm::quat orientation = getOrientation();
    view = glm::translate(glm::mat4(1.0f), position) * glm::toMat4(orientation);
    view = glm::inverse(view);
}

glm::vec2 FreeCamera::getPanSpeed() const {
    float x = std::min(m_ViewportWidth / 1000.0f, 2.4f);  // max = 2.4f
    float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;
    float y = std::min(m_ViewportHeight / 1000.0f, 2.4f);  // max = 2.4f
    float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;
    return {xFactor, yFactor};
}

float FreeCamera::RotationSpeed() const { return 0.8f; }

float FreeCamera::getZoomSpeed() const {
    float d = distance * 0.2f;
    d = std::max(d, 0.0f);
    float speed = d * d;
    speed = std::min(speed, 100.0f);  // max speed = 100
    return speed;
}

void FreeCamera::onUpdate(Time dt) {
    // TODO replace with key mappings
    if (Input::isKeyPressed(Key::LeftAlt)) {
        const glm::vec2 camSpeed = glm::vec2{10.f};
        if (Input::isKeyPressed(Key::KeyCode::Left)) {
            pan(glm::vec2{+camSpeed.x * dt, 0.f});
        }
        if (Input::isKeyPressed(Key::KeyCode::Right)) {
            pan(glm::vec2{-camSpeed.x * dt, 0.f});
        }
        if (Input::isKeyPressed(Key::KeyCode::Down)) {
            pan(glm::vec2{0.f, -camSpeed.y * dt});
        }
        if (Input::isKeyPressed(Key::KeyCode::Up)) {
            pan(glm::vec2{0.f, camSpeed.y * dt});
        }

        const glm::vec2& mouse{Input::getMouseX(), Input::getMouseY()};
        glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
        m_InitialMousePosition = mouse;

        if (Input::isMouseButtonPressed(Mouse::ButtonMiddle))
            pan(delta);
        else if (Input::isMouseButtonPressed(Mouse::ButtonLeft))
            MouseRotate(delta);
        else if (Input::isMouseButtonPressed(Mouse::ButtonRight))
            setZoomLevel(delta.y);
    }

    UpdateView();
}

void FreeCamera::onEvent(Event& e) {
    EventDispatcher dispatcher(e);
    dispatcher.dispatch<KeyPressedEvent>(
        std::bind(&FreeCamera::onKeyPressed, this, std::placeholders::_1));
    dispatcher.dispatch<Mouse::MouseScrolledEvent>(
        std::bind(&FreeCamera::onMouseScroll, this, std::placeholders::_1));
}

bool FreeCamera::onKeyPressed(KeyPressedEvent&) {
    if (!Input::isKeyPressed(Key::KeyCode::LeftAlt)) {
        return false;
    }
    return true;
}

bool FreeCamera::onMouseScroll(Mouse::MouseScrolledEvent& e) {
    float delta = e.GetYOffset() * 0.25f;
    setZoomLevel(delta);
    UpdateView();
    return false;
}

void FreeCamera::pan(const glm::vec2& delta) {
    auto s = getPanSpeed();
    focalPoint += -getRightDirection() * delta.x * s.x * distance;
    focalPoint += getUpDirection() * delta.y * s.y * distance;
}

void FreeCamera::MouseRotate(const glm::vec2& delta) {
    float yawSign = getUpDirection().y < 0 ? -1.0f : 1.0f;
    yaw += yawSign * delta.x * RotationSpeed();
    pitch += delta.y * RotationSpeed();
}

void FreeCamera::setZoomLevel(float delta) {
    distance -= delta * getZoomSpeed();
    if (distance < 1.0f) {
        focalPoint += getForwardDirection();
        distance = 1.0f;
    }
}
glm::vec3 FreeCamera::calculatePosition() const {
    return focalPoint - getForwardDirection() * distance;
}

glm::vec3 FreeCamera::getUpDirection() const {
    return glm::rotate(getOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 FreeCamera::getRightDirection() const {
    return glm::rotate(getOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 FreeCamera::getForwardDirection() const {
    return glm::rotate(getOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::quat FreeCamera::getOrientation() const {
    return glm::quat(glm::vec3(-pitch, -yaw, 0.0f));
}

