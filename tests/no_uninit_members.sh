#!/bin/sh
# CORRECT rule: every scalar member gets NSDMI. Fails on the a95712d shape.
set -e
if grep -nE '^\s+(bool|int|float) (lmouseDown|key|mod|keychar|modchar|yscrolled|isMinimized|running);' engine/ui.h engine/app.h; then echo "FAIL: uninitialized scalar member"; exit 1; fi
echo "no_uninit_members PASS"
