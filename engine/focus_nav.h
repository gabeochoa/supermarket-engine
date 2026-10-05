#pragma once
#include "uuid.h"
// Single source of tab-focus movement. CORRECT rule: widgets must call
// handle_tabbing() (which delegates here), never hand-roll tab/shift-tab.
// Past fixes re-implemented this per widget: 56c50ae, 2b3f156, b82756b, f2ee2b6, c741443.
namespace GOUI {
// Tab from `focused`: clear focus (next widget in call order grabs it).
// Shift-Tab (mod held): go back to the widget processed just before it.
inline uuid focus_after_tab(const uuid& /*focused*/, const uuid& lastProcessed, const uuid& root, bool modHeld) {
    return modHeld ? lastProcessed : root;
}
}  // namespace GOUI
