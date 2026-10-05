# CORRECT — supermarket-engine
Classes = recurrences ≥2 in `git log`/code. Full build pre-existing broken on this Mac (`GL/glew.h` not found), so proofs are standalone tests compiled with clang++ — stated honestly, not hidden.

| # | Class (evidence ×2+) | Level & why | Commit | Proof |
|---|---|---|---|---|
| 1 | Widget-id collisions: folded-xor hash was equality — `16346b1` loop macro :facepalm:, `16394ac` add parent hash, `b9c68ca` broken uuid comparisons, `51adb78` "uuid system is still not good" | Types/architecture: identity = tuple fields, `hash` is bucket-only; no call-site discipline can fix a colliding equality | 9a6350c | `tests/uuid_test.cpp` PASS; old header on line2/- vs line0/idx1 pair prints COLLIDE (both old-hash 8) |
| 2 | Tab/shift-tab re-fixed per widget: `56c50ae` commandline, `2b3f156` shift+tab, `b82756b` dropdown, `f2ee2b6` textfield→handle_tabbing, `c741443` slider→handle_tabbing, `9198449` tab-shouldnt-select | Architecture: one pure `focus_after_tab()` in `focus_nav.h`; `handle_tabbing` delegates, widgets never hand-roll | 8f32f9c + fixup | `tests/focus_nav_test.cpp` PASS (tab→root, shift-tab→lastProcessed) |
| 3 | Uninitialized scalars (UB): `a95712d` "werent init this bool", same shape left in UIContext `lmouseDown/key/mod/yscrolled`, `FontPhraseTexInfo::valid()` reading uninit fields, App `running/width` | Types (NSDMI at declaration — constructor checklists were the failed level) + lint guard naming the fix | cc7ce3d | `tests/no_uninit_members.sh` FAIL(1) on pre-fix tree, PASS after |

Not fixed (recorded, not invented): direct patch to `vendor/fmt/core.h` (pre-existing uncommitted owner change, left untouched) — rule: patch vendor only via a named, re-appliable patch file, never silently (cf. README "@SUPERMARKET" edits in backward-cpp).

## Rule table (also for the next agent)
| Rule | Enforcement |
|---|---|
| Identity is the uuid tuple; never compare/hash-fold raw fields | `engine/uuid.h` types + `tests/uuid_test.cpp` |
| Tab movement only via `handle_tabbing`→`focus_after_tab`; composite widgets delegate to `button*` children | `engine/focus_nav.h` + `tests/focus_nav_test.cpp` |
| Every scalar member: NSDMI `=false/0/0.f/{}` at declaration | `tests/no_uninit_members.sh` |
| Stateful widget in a loop: `MK_UUID_LOOP`, never plain `MK_UUID` | uuid tuple includes `index`; test asserts index distinguishes |
