
#pragma once

#include "../engine/pch.hpp"

const std::array<glm::vec2, 4> getBoundingBox(glm::vec2 position,
                                              glm::vec2 size) {
    return {position,
            {position.x + size.x, position.y + 0},
            {position.x, position.y + size.y},
            {position.x + size.x, position.y + size.y}};
}

static int ENTITY_ID_GEN = 0;
struct Entity {
    int id;
    glm::vec2 position;
    glm::vec2 size;
    float angle;
    glm::vec4 color;
    std::string textureName;
    bool center = true;

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

    virtual void onUpdate(Time dt) { (void)dt; }

    virtual bool pointCollides(const glm::vec2& m) const {
        auto [a, b, c, d] = getBoundingBox(position, size);

        auto ab = b - a;
        auto am = m - a;
        auto bc = c - b;
        auto bm = m - b;

        auto abam = glm::dot(ab, am);
        auto abab = glm::dot(ab, ab);
        auto bcbm = glm::dot(bc, bm);
        auto bcbc = glm::dot(bc, bc);

        return (             //
            0 <= abam &&     //
            abam <= abab &&  //
            0 < bcbm &&      //
            bcbm <= bcbc     //
        );                   //
    }

    bool entityCollides(const std::shared_ptr<Entity>& other) const {
        // collision x-axis?
        bool collisionX =
            this->position.x + this->size.x >= other->position.x &&
            other->position.x + other->size.x >= this->position.x;
        // collision y-axis?
        bool collisionY =
            this->position.y + this->size.y >= other->position.y &&
            other->position.y + other->size.y >= this->position.y;
        // collision only if on both axes
        return collisionX && collisionY;
    }

    virtual void render() {
        // computing angle transforms are expensive so
        // if the angle is under thresh, just render it square
        if (angle <= 5.f) {
            auto loc = glm::vec2{
                position.x,
                position.y,
            };
            if (center) {
                loc = loc + glm::vec2{size.x / 2, size.y / 2};
            }
            Renderer::drawQuad(loc, size, color, textureName);
        } else {
            Renderer::drawQuadRotated(position, size, glm::radians(angle),
                                      color, textureName);
        }
    }

    virtual const Entity* asEntity() const { return this; }
    void announce(const std::string& tosay) const;

    virtual const char* typeString() const = 0;
};

template <>
struct fmt::formatter<Entity> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    constexpr auto format(Entity const& e, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}{} at {}", e.typeString(), e.id,
                              e.position);
    }
};

void Entity::announce(const std::string& tosay) const {
    log_info("{}: {}", *asEntity(), tosay);
}

static std::vector<std::shared_ptr<Entity>> entities;
