# Multi-agent project rules

Codex and Claude work in separate worktrees. Do not edit the other agent's worktree or send input to its running game.

## Ownership

| Owner | Files |
| --- | --- |
| Codex | `prototype/engine/bridge.c`, `prototype/engine/server.js`, `prototype/ground-notice.js`, `prototype/combat-visuals.js`, `prototype/equipment.js`, tests, and gameplay code in `src/` |
| Claude | `prototype/creatures.js`, `prototype/floor.js`, `prototype/cavern.js`, `prototype/readability.js`, `prototype/fire.js`, `prototype/altar.js` |
| Shared | `prototype/live.js`, `prototype/main.js`, `prototype/style.css` |

Before editing a shared file, read and append an entry to `/Users/dpalm/Desktop/deanhack-handoff.md` describing the planned change. Check the log for the other agent's recent entries first.

When finishing work, run relevant tests/builds, commit on the agent's branch, and append files changed, checks run, commit hash, and open concerns to the handoff log.

Do not retune `LIVE_AMBIENT`, `TORCH_INTENSITY`, or lantern lighting without documenting the reason in the handoff log. These settings are deliberate contrast fixes.

The live engine is shared with the user's running game. Never send input to it for testing; use `prototype/engine/smoke.py` and its separate character instead.

Merge one branch at a time. After a merge, the other agent must update its branch before beginning another task. For conflict-prone shared files, only one agent edits at a time.
