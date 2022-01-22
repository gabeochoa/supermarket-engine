
#include "keycodes.h"

#include "edit.h"

namespace Key {

void initMapping() { GLOBALS.set<Mapping>("keymapping", new Mapping()); }

KeyCode getMapping(const char* keyname) {
    auto map = GLOBALS.get_ptr<Mapping>("keymapping");
    return map->mapping[keyname];
}

}  // namespace Key
