# Kek Raylib Game Example

This example is a small top-down arena shooter built on the project schema generator plus the generated Kek runtime wrapper.

## Build

```sh
make game
make run-game
```

`make game` regenerates `generated/game_state.c`, builds the Kek runtime library, extracts the vendored raylib archive for the host OS, and links `build/game` against the vendored static raylib library.

On Linux, the Makefile uses `raylib-5.5_linux_amd64.tar.gz` and links with X11/OpenGL system libraries such as `libX11`, `libGL`, `libxcb`, `libXau`, and `libXdmcp`. Minimal containers may compile the game objects successfully but fail the final link until those packages are installed.

On macOS, the Makefile uses `raylib-5.5_macos.tar.gz` and links with the native CoreVideo, IOKit, Cocoa, GLUT, and OpenGL frameworks.

## Controls

- `WASD`: move
- Mouse or arrow keys: aim
- Left mouse or `Space`: shoot
- Right mouse or `Left Shift`: dash
- `P` or `Esc`: pause/resume
- `F3`: debug overlay
- `1`, `2`, `3`: choose upgrades

## State Model

`game.schema.json` declares the generated states:

- `GameSession`: menu/play/pause/upgrade/game-over/victory mode, score, combo, timers, shake, debug flag.
- `Player`: position, velocity, health, shield, level, cooldowns, invulnerability.
- `FrameClock`: Raylib frame tick and delta time, written once per playing frame.
- `WaveDirector`: current wave, spawn budget, spawn timer, active enemy count, boss state.
- `InputIntent`: normalized input snapshot written every frame.
- `CameraRig`: camera smoothing and zoom.
- Named `Enemy` instances for the showcase grunt, runner, tank, and boss.
- Dynamic `Enemy`, `Projectile`, `Pickup`, and `HudMessage` slots for wave spawns and temporary entities.

Hooks generated from the schema are implemented in `game_hooks.inc.c`, which is included by `main.c`:

- `OnFrameClock` advances waves when the arena is clear.
- `MoveGruntEnemy`, `MoveRunnerEnemy`, `MoveTankEnemy`, and `MoveBossEnemy` move the four named enemy instances with different movement parameters.
- `OnPlayerHealthChanged` moves the session to game-over.
- `OnWaveChanged` moves the session to victory.
- `OnScoreChanged` opens an upgrade state at score thresholds.

The raylib code handles input, drawing, and writes `FrameClock` from `GetFrameTime()`. Authoritative game data lives in generated state slots. `main.c` owns one `Game_stateRuntime`, which initializes the runtime, generated store, declared slots, and hooks with one call. Gameplay code uses generated runtime-scoped create/delete/find/update helpers instead of manually passing `KekStateStore` pointers and declared slot ids around.
