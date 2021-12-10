
#pragma once

#include "../engine/pch.hpp"

static int ENTITY_ID_GEN = 0;
struct Entity {
    int id;
    glm::vec2 position;
    glm::vec2 size;
    float angle;
    glm::vec4 color;
    std::string textureName;

    Entity()
        : id(ENTITY_ID_GEN),
          position({0.f, 0.f}),
          size({1.f, 1.f}),
          angle(0.f),
          color({1.f, 1.f, 1.f, 1.f}),
          textureName("white") {}

    Entity(const glm::vec2& position_, const glm::vec2& size_, float angle_,
           const glm::vec4& color_, const std::string& textureName_)
        : position(position_),
          size(size_),
          angle(angle_),
          color(color_),
          textureName(textureName_) {}

    virtual ~Entity() {}

    virtual void onUpdate(Time dt) {}

    virtual void render() {
        // computing angle transforms are expensive so
        // if the angle is under thresh, just render it square
        if (angle <= 5.f) {
            Renderer::drawQuad(position, size, color, textureName);
        } else {
            Renderer::drawQuadRotated(position, size, glm::radians(angle),
                                      color, textureName);
        }
    }

    virtual const Entity* asEntity() const {
        log_warn("calling as ent");
        return this;
    }
    void announce(const std::string& tosay) const;
};

template <>
struct fmt::formatter<Entity> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(Entity const& e, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "Entity{} at {}", e.id, e.position);
    }
};

void Entity::announce(const std::string& tosay) const {
    log_info(fmt::format("{}: {}", *asEntity(), tosay));
}

