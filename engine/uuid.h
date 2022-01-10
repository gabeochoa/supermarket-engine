#pragma once

#include <atomic>
#include <iostream>

#include "constexpr_map.h"
#include "external_include.h"

namespace IUI {

// From cppreference.com
// For two different parameters k1 and k2 that are not equal, the probability
// that std::hash<Key>()(k1) == std::hash<Key>()(k2) should be very small,
// approaching 1.0/std::numeric_limits<std::size_t>::max().
//
// so basically dont have more than numeric_limits::max() ui items per layer
//
// thx

struct uuid {
    int owner;
    std::size_t hash;

    uuid() : uuid(-99, "__MAGIC__STRING__", -1) {}
    uuid(const std::string& s1, int i1) : uuid(-1, s1, i1) {}

    uuid(int o, const std::string& s1, int i1) {
        owner = o;
        auto h1 = std::hash<std::string>{}(s1);
        auto h2 = std::hash<int>{}(i1);
        hash = h1 ^ (h2 << 1);
    }

    uuid(int o, const std::string& s1, int i1, int index) {
        owner = o;
        auto h1 = std::hash<std::string>{}(s1);
        auto h2 = std::hash<int>{}(i1);
        auto h3 = std::hash<int>{}(index);
        hash = h1 ^ (h2 << 1) ^ (h3 << 2);
    }

    uuid(const uuid& other) { this->operator=(other); }

    uuid& operator=(const uuid& other) {
        this->owner = other.owner;
        this->hash = other.hash;
        return *this;
    }

    bool operator==(const uuid& other) const {
        return owner == other.owner && hash == other.hash;
    }

    bool operator<(const uuid& other) const {
        if (owner < other.owner) return true;
        if (owner > other.owner) return false;
        if (hash < other.hash) return true;
        if (hash > other.hash) return false;
        return false;
    }
};

std::ostream& operator<<(std::ostream& os, const uuid& obj) {
    os << fmt::format("owner: {} hash: {}", obj.owner, obj.hash);
    return os;
}

#define MK_UUID(x) uuid(x, __FILE__, __LINE__)
#define MK_UUID_LOOP(x, index) uuid(x, __FILE__, __LINE__, index)

static uuid rootID = MK_UUID(-1);
static uuid fakeID = MK_UUID(-2);

}  // namespace IUI

