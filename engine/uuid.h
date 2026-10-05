#pragma once
// Widget identity. CORRECT rule (see CORRECT.md): identity is the tuple
// (ownerLayer, parentHash, file, line, index) compared field-by-field.
// The old folded `h0 ^ (h1<<1) ^ (h2<<2) ^ (h3<<3)` hash was equality itself,
// so distinct tuples collided (e.g. line=2,index=0 vs line=0,index=1 both
// gave 8) and widgets silently shared state/focus. `hash` below is only a
// bucket hint for maps/parents, never identity.
#include <cstddef>
#include <ostream>
#include <string>

namespace GOUI {
inline std::size_t uuid_combine(std::size_t seed, std::size_t v) {
    return seed ^ (v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}
struct uuid {
    int ownerLayer = -99;
    std::size_t parentHash = 0;
    std::size_t fileHash = 0;
    int line = -1;
    int index = -1;
    std::size_t hash = 0;
    uuid() = default;
    uuid(const std::string& s1, int i1) : uuid(-1, 0, s1, i1) {}
    uuid(int layer, std::size_t parent, const std::string& s1, int i1, int idx = -1)
        : ownerLayer(layer), parentHash(parent), fileHash(std::hash<std::string>{}(s1)), line(i1), index(idx) {
        hash = uuid_combine(uuid_combine(uuid_combine(uuid_combine(
            std::hash<int>{}(ownerLayer), parentHash), fileHash),
            std::hash<int>{}(line)), std::hash<int>{}(index));
    }
    bool operator==(const uuid& o) const {
        return ownerLayer == o.ownerLayer && parentHash == o.parentHash &&
               fileHash == o.fileHash && line == o.line && index == o.index;
    }
    bool operator!=(const uuid& o) const { return !(*this == o); }
    bool operator<(const uuid& o) const {
        if (ownerLayer != o.ownerLayer) return ownerLayer < o.ownerLayer;
        if (parentHash != o.parentHash) return parentHash < o.parentHash;
        if (fileHash != o.fileHash) return fileHash < o.fileHash;
        if (line != o.line) return line < o.line;
        return index < o.index;
    }
    operator std::size_t() const { return hash; }
    operator std::string() const {
        return "layer: " + std::to_string(ownerLayer) + " hash: " + std::to_string(hash);
    }
};
inline std::ostream& operator<<(std::ostream& os, const uuid& obj) { return os << std::string(obj); }
#define MK_UUID(x, parent) uuid(x, parent, __FILE__, __LINE__)
#define MK_UUID_LOOP(x, parent, index) uuid(x, parent, __FILE__, __LINE__, index)
static uuid rootID = MK_UUID(-1, -1);
static uuid fakeID = MK_UUID(-2, -1);
}  // namespace GOUI
