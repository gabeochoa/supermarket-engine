
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
};

struct Item : public Entity {
    const char* name;
    const double price;

    Item(const char* n, const double p) : name(n), price(p) {}
};

struct ItemGroup {
    std::unordered_map<std::string, int> group;

    void addItem(const std::string& name, int amount) {
        if (group.find(name) == group.end()) {
            group[name] = 0;
        }
        group[name] += amount;
    }

    int removeItem(const std::string& name, int amount) {
        auto it = group.find(name);
        if (it == group.end()) {
            log_warn(fmt::format(
                "Trying to remove {} of {} but this ItemGroup doesnt have that",
                amount, name));
            return 0;
        }
        if (it->second >= amount) {
            group[name] -= amount;
            return amount;
        }
        int has = group[name];
        group[name] = 0;
        return has;
    }
};

struct Shelf : public Entity {
    ItemGroup contents;

    Shelf(const glm::vec2& position, const glm::vec2& size, float angle,
          const glm::vec4& color, const std::string& textureName)
        : Entity(position, size, angle, color, textureName) {}

    virtual ~Shelf() {}
};

#include "movable_entities.h"
