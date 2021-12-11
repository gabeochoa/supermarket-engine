#pragma once

#include "../engine/pch.hpp"
#include "../vendor/fmt/ostream.h"
#include "entity.h"

struct Item {
    const char* name;
    const float price;
    // TODO eventually should be a string with texture, but for now, color
    const glm::vec4 color;
    constexpr Item(const char* n, const float p, const glm::vec4& c)
        : name(n), price(p), color(c) {}
};

// error: excess items in array initializer
//             number of items should match       V
static constexpr std::array<std::pair<int, Item>, 4> items_{
    //
    {
        {0, {"apple", 1.f, glm::vec4{1.f}}},  //
        {1, {"apple", 1.f, glm::vec4{1.f}}},  //
        {2, {"apple", 1.f, glm::vec4{1.f}}},  //
        {3, {"apple", 1.f, glm::vec4{1.f}}},  //
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
            log_warn(fmt::format(
                "Trying to remove {} of {} but this ItemGroup doesnt have that",
                amount, itemID));
            return 0;
        }
        if (it->second >= amount) {
            group[itemID] -= amount;
            return amount;
        }
        int has = group[itemID];
        group[itemID] = 0;
        return has;
    }

    int operator[](int id) const { return group.find(id)->second; }
    auto size() { return group.size(); }
    auto begin() { return group.begin(); }
    auto end() { return group.end(); }

    auto begin() const { return group.begin(); }
    auto end() const { return group.end(); }

    friend std::ostream& operator<<(std::ostream& os, const ItemGroup& ig) {
        for (auto& kv : ig.group) {
            os << "(" << kv.first << "," << kv.second << ")"
               << "\n";
        }
        return os;
    }
};

struct Shelf : public Entity {
    static constexpr std::array<std::pair<float, float>, 4> item_positions{{
        //
        {0.11f, 0.1f},
        {0.51f, 0.1f},
        {0.11f, 0.5f},
        {0.51f, 0.5f},
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

    Shelf(const glm::vec2& position, const glm::vec2& size, float angle,
          const glm::vec4& color, const std::string& textureName)
        : Entity(position, size, angle, color, textureName) {}

    virtual ~Shelf() {}
    virtual const char* typeString() const override { return "Shelf"; }
    virtual void render() override {
        if (contents.size() > 4) {
            log_warn("Contents is too large and so not all items will display");
        }
        int index = 0;
        for (auto& kv : contents) {
            if (index >= 4) break;
            // TODO replace "face" with the real texture
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
                    {0.1f, 0.1f},                         // size
                    item.color,                           // color
                    "face"                                // tex
                );
            }
            index++;
        }

        Entity::render();
    }

    static std::vector<std::shared_ptr<Shelf>> getShelvesInRange(glm::vec2 pos,
                                                                 float range) {
        std::vector<std::shared_ptr<Shelf>> shelves;
        for (auto& e : entities) {
            auto s = dynamic_pointer_cast<Shelf>(e);
            if (!s) continue;
            if (glm::distance(pos, e->position) < range) {
                shelves.push_back(s);
            }
        }
        return shelves;
    }
};

