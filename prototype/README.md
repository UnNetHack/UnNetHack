# The Quiet Well / Live UnNetHack

Two modes share the 3D presentation:

- **Demo room** is the original independent diorama simulation.
- **Live UnNetHack** runs the repository's C game with an experimental JSON window
  port. Movement, pet AI, combat, visibility, inventory and turns belong to the engine.

The browser is a local development preview, not the final native client.

## Run

Requires Node.js 22.12+ (or a supported newer version), Python 3.9+, a C toolchain,
make, pkg-config, and Lua development headers. The engine build currently targets
macOS, matching this repository's local setup.

```sh
cd /Users/dpalm/Desktop/deanhack/prototype
npm ci
npm run engine:build
npm run dev
```

Open the printed localhost URL and select **Live UnNetHack**. The initial character
is a lawful human Valkyrie named Wanderer with a cat. A saved Wanderer resumes on
starting Live mode again. Switching to Demo room pauses input to the existing engine;
it does not create or reset a character. A page reload likewise does not reset the
engine: select Live UnNetHack again to reconnect to its pending prompt.

`h j k l` move left/down/up/right; `y u b n` move diagonally. Arrows also move.
`s` searches; `o` opens a door; Space waits; `i` opens inventory; `<` / `>` use stairs. Toolbar
buttons expose pickup, open, quaff and save. Engine questions open a dialog; use
arrows or the original h/j/k/l keys for direction prompts. Menu selections and text
are sent only in response to a matching engine request. Directions remain relative
to the map when rotating the camera. Drag to orbit and scroll to zoom.

**Save & exit** asks the engine to save. Wait for the session-ended message. Switching
modes is not a save command. There is no automatic turn advance while reading a prompt.

## Isolation and build behavior

Everything the engine writes lives under `prototype/.engine/runtime`, including
saves, level files, bones, logs, and its separate HOME. The helper copies source into
`.engine/source`, configures explicit paths, verifies the generated path header,
and builds there. It never invokes `make install` or copies existing live saves.
Do not delete `.engine` if you want to retain these experimental characters.

The bridge is enabled by `BRIDGE_GRAPHICS` only in this isolated build. The normal
TTY/SDL build keeps its existing window ports. Unix startup now skips terminal
probing when stdin/stdout are pipes.

Subsequent builds retain objects and skip unchanged configure inputs. C compilation
is parallel; legacy level-data targets are serialized to avoid concurrent compiler
rewrites. Commands have time and output limits; logs are in `.engine`. Executables
and runtime data are replaced atomically so a running process retains its old open
files. Save and resume to use a new engine binary. Future engine/save-format changes
still require explicit migration planning.

## Boundary

```text
Browser input → local Vite middleware → engine stdin
UnNetHack window callbacks → JSON stdout → server events → Three.js scene
```

`engine/bridge.c` buffers emitted glyphs and sends a frame before each input request.
Terrain under entities uses the window-port background convention; background updates
are retained only while the square is visible. It does not enumerate hidden monsters
or unexplored terrain. Creature names describe the displayed glyph, including the
engine's hallucination substitutions. There is no renderer-side collision or damage
prediction in Live mode.

`engine/server.js` launches one isolated process. It validates same-origin loopback
requests, a session token, request IDs and bounded key/menu/line replies. Stale input
cannot consume a later turn. `live.js` renders observations, preserving known terrain
between inputs and animating visual motion independently of game turns.

## Validation

```sh
npm test                  # simulation and protocol validation
npm run build             # frontend production compilation
python3 engine/smoke.py    # real engine: move, inventory, save
python3 engine/smoke.py    # restore that test character, repeat
```

The smoke test uses a separate BridgeSmoke character in the isolated runtime. It
checks that movement advances time, inventory does not, ground is present beneath
the player, and a save exists. It never drives the browser's Wanderer character.

## Current limits

This is an engine-integration prototype, not a complete window-port replacement.
It has no native packaging, character-creation UI, positional targeting UI, message
history browser, or per-monster imported art. Cats use the animated cat model; other
creatures use a labeled placeholder. Other objects/features retain symbolic markers.
Visual entity matching uses glyph/proximity rather than persistent engine IDs.
Menus support selections but not quantity counts, preselection or group accelerators;
text/menu buffers are limited to 512 entries. Persistent inventory, rich condition
icons, reliable attack-event animation, and exhaustive command/endgame coverage remain
future work. These limits do not change the underlying engine's rules.

All models are procedural geometry with animated joint hierarchies, not imported
skeletal clips. The optional Google Fonts request falls back to local system fonts.
A production frontend build alone does not include the local engine service: use
`npm run dev` for Live mode until native/service packaging is implemented.
