#include "../engine/uuid.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace GOUI;
    // Past mistake (old folded hash): these two distinct widgets collided at 8.
    uuid a(0, 0, "f", 2, -1), b(0, 0, "f", 0, 1);
    assert(a != b); assert(a < b || b < a);
    // Loop widgets on one line must differ only via MK_UUID_LOOP index.
    assert(uuid(0,0,"f",7,0) != uuid(0,0,"f",7,1));
    // Same call site in different parents/layers must not share state.
    assert(uuid(1,0,"f",7) != uuid(2,0,"f",7));
    assert(uuid(1,111,"f",7) != uuid(1,222,"f",7));
    // Deterministic identity for the same tuple.
    assert(uuid(1,111,"f",7,3) == uuid(1,111,"f",7,3));
    std::cout << "uuid_test PASS\n";
}
