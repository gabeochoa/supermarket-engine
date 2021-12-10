
#pragma once

#include "../engine/pch.hpp"
#include "entity.h"

struct Billboard : public Entity {
    // Billboard is a textured ent that never moves
    Billboard(const glm::vec2& position, const glm::vec2& size, float angle,
              const glm::vec4& color,
              // TODO somehow i broke colored textures...
              // so for now we will use a definite undefinied tex
              // and it will trigger the flat shader
              const std::string& textureName = "__INVALID__")
        : Entity(position, size, angle, color, textureName) {}

    virtual ~Billboard() {}
    virtual const char* typeString() const override { return "Billboard"; }
};

#include "item.h"
#include "movable_entities.h"
