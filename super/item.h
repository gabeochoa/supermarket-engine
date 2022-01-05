#pragma once

#include "../engine/pch.hpp"
#include "../engine/renderer.h"
#include "../vendor/fmt/ostream.h"
//
#include "entity.h"

struct Item {
    const char* name;
    const float price;
    const char* textureName;
    const glm::vec4 color;

    constexpr Item(const char* n, const float p, const char* t,
                   const glm::vec4& c)
        : name(n), price(p), textureName(t), color(c) {}
};

// error: excess items in array initializer
//             number of items should match       V
static constexpr std::array<std::pair<int, Item>, 4> items_{
    //
    {
        {0, {"egg", 1.f, "egg", glm::vec4{1.f}}},                    //
        {1, {"milk", 1.f, "milk", glm::vec4{1.f}}},                  //
        {2, {"peanutbutter", 1.f, "peanutbutter", glm::vec4{1.f}}},  //
        {3, {"pizza", 1.f, "pizza", glm::vec4{1.f}}},                //
    }
    //
};

struct ItemManager {
    static void init_items() {
        // TODO read from a file;
    }
    static Item getItem(int id) {
        static constexpr auto ALL_ITEMS =
            CEMap<int, Item, items_.size()>{{items_}};
        return ALL_ITEMS.at(id);
    }
};

struct ItemGroup {
    std::map<int, int> group;

    void addItem(int itemID, int amount) {
        if (group.find(itemID) == group.end()) {
            group[itemID] = 0;
        }
        group[itemID] += amount;
    }

    // returns the amount removed
    int removeItem(int itemID, int amount) {
        auto it = group.find(itemID);
        if (it == group.end()) {
            log_warn(
                "Trying to remove {} of {} but this ItemGroup doesnt have that",
                amount, itemID);
            return 0;
        }
        if (it->second >= amount) {
            group[itemID] -= amount;
            return amount;
        }
        int has = group[itemID];
        group.erase(itemID);
        return has;
    }

    int operator[](int id) const { return group.find(id)->second; }
    auto size() { return group.size(); }
    auto begin() { return group.begin(); }
    auto end() { return group.end(); }

    auto begin() const { return group.begin(); }
    auto end() const { return group.end(); }

    auto rbegin() const { return group.rbegin(); }
    auto rend() const { return group.rend(); }

    auto rbegin() { return group.rbegin(); }
    auto rend() { return group.rend(); }

    auto empty() const { return group.empty(); }
    auto find(int id) const { return group.find(id); }

    friend std::ostream& operator<<(std::ostream& os, const ItemGroup& ig) {
        for (auto& kv : ig.group) {
            os << "(" << kv.first << "," << kv.second << ")"
               << "\n";
        }
        return os;
    }
};

struct Storable : public Entity {
    static constexpr std::array<std::pair<float, float>, 4> item_positions{{
        //
        {0.20f, 0.2f},
        {0.60f, 0.2f},
        {0.20f, 0.6f},
        {0.60f, 0.6f},
        //
    }};
    static constexpr std::array<std::pair<float, float>, 9> item_offsets{{
        //
        {0.0f, 0.0f},
        {0.0f, 0.1f},
        {0.0f, 0.2f},

        {0.1f, 0.0f},
        {0.1f, 0.1f},
        {0.1f, 0.2f},

        {0.2f, 0.0f},
        {0.2f, 0.1f},
        {0.2f, 0.2f},
        //
    }};

    ItemGroup contents;

    Storable(const glm::vec2& position, const glm::vec2& size, float angle,
             const glm::vec4& color, const std::string& textureName)
        : Entity(position, size, angle, color, textureName) {}

    virtual void render() override {
        Entity::render();

        if (contents.size() > 4) {
            log_warn("Contents is too large and so not all items will display");
        }
        int index = 0;
        for (auto& kv : contents) {
            if (index >= 4) break;
            Item item = ItemManager::getItem(kv.first);
            auto basepos =
                glm::vec3{position.x + item_positions[index].first,
                          position.y + item_positions[index].second, 0.f};
            for (int i = 0; i < kv.second; i++) {
                if (i >= 9) break;
                Renderer::drawQuad(                       //
                    {basepos.x + item_offsets[i].first,   // pos
                     basepos.y + item_offsets[i].second,  // pos
                     basepos.z},                          // pos
                    {0.2f, 0.2f},                         // size
                    item.color,                           // color
                    item.textureName                      // textureName
                );
            }
            index++;
        }
    }

    template <typename T>
    static std::shared_ptr<T> getRandomStorage(const glm::vec2& notpos = {
                                                   -99.f, -99.f}) {
        std::vector<std::shared_ptr<T>> matching;
        for (auto& e : entities) {
            auto s = dynamic_pointer_cast<T>(e);
            if (!s) continue;
            if (glm::distance(s->position, notpos) > 1.f) matching.push_back(s);
        }
        int i = randIn(0, matching.size() - 1);
        return matching[i];
    }

    template <typename T>
    static std::vector<std::shared_ptr<T>> getStorageInRange(glm::vec2 pos,
                                                             float range) {
        std::vector<std::shared_ptr<T>> matching;
        for (auto& e : entities) {
            auto s = dynamic_pointer_cast<T>(e);
            if (!s) continue;
            if (glm::distance(pos, e->position) < range) {
                matching.push_back(s);
            }
        }
        return matching;
    }

    template <typename T>
    static std::vector<std::shared_ptr<T>> getStorageInRangeWithItem(
        glm::vec2 pos, int itemID, float range) {
        std::vector<std::shared_ptr<T>> matching;
        for (auto& e : entities) {
            auto s = dynamic_pointer_cast<T>(e);
            if (!s) continue;
            if (glm::distance(pos, e->position) > range) continue;
            if (s->contents.find(itemID) != s->contents.end()) {
                matching.push_back(s);
            }
        }
        return matching;
    }

    template <typename T>
    static std::vector<std::shared_ptr<T>> getStorageInSelection(
        glm::vec4 rect) {
        std::vector<std::shared_ptr<T>> matching;
        for (auto& e : entities) {
            auto s = dynamic_pointer_cast<T>(e);
            if (!s) continue;
            if (aabb(s->getRect(), rect)) {
                matching.push_back(s);
            }
        }
        return matching;
    }
};

struct Storage : public Storable {
    virtual const char* typeString() const override { return "Storage"; }

    Storage(const glm::vec2& position, const glm::vec2& size, float angle,
            const glm::vec4& color, const std::string& textureName)
        : Storable(position, size, angle, color, textureName) {}
};

struct Shelf : public Storable {
    virtual const char* typeString() const override { return "Shelf"; }

    Shelf(const glm::vec2& position, const glm::vec2& size, float angle,
          const glm::vec4& color, const std::string& textureName)
        : Storable(position, size, angle, color, textureName) {}
};

