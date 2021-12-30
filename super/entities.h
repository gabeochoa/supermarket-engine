
#pragma once

#include "../engine/pch.hpp"
#include "entity.h"

struct Billboard : public Entity {
    // Billboard is a textured ent that never moves
    Billboard(const glm::vec2& position, const glm::vec2& size, float angle,
              const glm::vec4& color, const std::string& textureName = "white")
        : Entity(position, size, angle, color, textureName) {}

    virtual ~Billboard() {}
    virtual const char* typeString() const override { return "Billboard"; }
};

struct DragArea : public Entity {
    bool isMouseDragging = false;
    glm::vec2 mouseDragStart;
    glm::vec2 mouseDragEnd;

    DragArea(const glm::vec2& position, const glm::vec2& size, float angle,
             const glm::vec4& color, const std::string& textureName = "white")
        : Entity(position, size, angle, color, textureName) {}

    virtual ~DragArea() {}
    virtual const char* typeString() const override { return "DragArea"; }

    virtual void onUpdate(Time dt) override {
        (void)dt;
        // TODO figure out a way to lock on tiles round up?
        position = mouseDragStart;
        size = (mouseDragEnd - mouseDragStart);
    }
};

#include "item.h"
#include "movable_entities.h"
