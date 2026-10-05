#include "../engine/focus_nav.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace GOUI;
    uuid root(-1, 0, "r", 0), prev(0, 0, "f", 10), cur(0, 0, "f", 20);
    assert(focus_after_tab(cur, prev, root, false) == root);  // tab: hand off (56c50ae)
    assert(focus_after_tab(cur, prev, root, true) == prev);   // shift-tab: back (2b3f156)
    std::cout << "focus_nav_test PASS\n";
}
