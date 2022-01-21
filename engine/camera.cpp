
#include "camera.h"

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
        if (Input::isKeyPressed(Key::mapping["Left"])) {
            this->camera.position.x -= camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Right"])) {
            this->camera.position.x += camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Down"])) {
            this->camera.position.y -= camSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Up"])) {
            this->camera.position.y += camSpeed * dt;
        }
    }
    if (rotationEnabled) {
        if (Input::isKeyPressed(Key::mapping["Rotate Clockwise"])) {
            this->camera.rotation += rotSpeed * dt;
        }
        if (Input::isKeyPressed(Key::mapping["Rotate Counterclockwise"])) {
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
    float zl = fmin(fmax(zoomLevel - (event.GetYOffset() * 0.25), 0.5f), 20.f);
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
