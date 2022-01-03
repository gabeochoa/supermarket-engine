#pragma once

#include <iostream>

#include "external_include.h"

namespace IUI {
struct uuid {
    int owner;
    int item;
    int index;

    bool operator==(const uuid& other) const {
        return owner == other.owner && item == other.item &&
               index == other.index;
    }

    bool operator<(const uuid& other) const {
        if (owner < other.owner) return true;
        if (owner > other.owner) return false;
        if (item < other.item) return true;
        if (item > other.item) return false;
        if (index < other.index) return true;
        if (index > other.index) return false;
        return false;
    }
};
std::ostream& operator<<(std::ostream& os, const uuid& obj) {
    os << fmt::format("owner: {} item: {} index: {}", obj.owner, obj.item,
                      obj.index);
    return os;
}

static uuid rootID = uuid({.owner = -1, .item = 0, .index = 0});
static uuid fakeID = uuid({.owner = -2, .item = 0, .index = 0});

}  // namespace IUI

