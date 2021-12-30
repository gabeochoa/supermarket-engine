
#pragma once

#include "../engine/pch.hpp"
#include "entities.h"
#include "movable_entities.h"

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
        position = round_higher(mouseDragStart);
        size = round_higher(mouseDragEnd - mouseDragStart);

        if (!isMouseDragging && glm::length(size) > 1.f) {
            auto a = mouseDragStart;
            auto b = mouseDragEnd;

            // we need to figure out the rect from the
            // bottom left to top right so finding works correctly
            // theres 4 ways you can drag the box out
            //  1. top left to bottom right
            //  2. top right to bottom left
            //  3. bottom left to top right
            //  4. bottom right to top left

            auto rect = glm::vec4{0.f};
            if (0) {
            } else if (a.x <= b.x && a.y >= b.y) {
                rect = glm::vec4{a.x, b.y, b.x, a.y};
            } else if (a.x >= b.x && a.y >= b.y) {
                rect = glm::vec4{b.x, b.y, a.x, a.y};
            } else if (a.x <= b.x && a.y <= b.y) {
                rect = glm::vec4{a, b};
            } else if (a.x >= b.x && a.y <= b.y) {
                rect = glm::vec4{b.x, a.y, a.x, b.y};
            }

            auto furniture = Storable::getStorageInSelection<Storable>(rect);

            log_info("got {} furnitues", furniture.size());

            // clear drag area
            size = glm::vec2{0.f};
            position = glm::vec2{0.f};
            mouseDragStart = glm::vec2{0.f};
            mouseDragEnd = glm::vec2{0.f};
        }
    }
};

