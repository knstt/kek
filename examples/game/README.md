# Kek Raylib Game Example

This example is a small top-down arena shooter built on the project schema generator plus the Kek runtime state store.

## Build

```sh
make game
make run-game
```

`make game` regenerates `generated/game_state.c`, extracts `raylib-6.0_linux_amd64.tar.gz`, builds the Kek runtime library, and links `build/game` against the vendored static raylib library.

On Linux, the static raylib link also needs system graphics development libraries such as `libX11` and `libGL`. Minimal containers may compile the game objects successfully but fail the final link until those packages are installed.

The folder also contains the upstream `raylib-6.0.tar.gz` source archive and the ARM64 binary archive, but the Makefile target uses the AMD64 archive for this host.

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
- `WaveDirector`: current wave, spawn budget, spawn timer, active enemy count, boss state.
- `InputIntent`: normalized input snapshot written every frame.
- `CameraRig`: camera smoothing and zoom.
- Dynamic `Enemy`, `Projectile`, `Pickup`, and `HudMessage` slots.
- Standard `Timer`, driven manually from raylib frame time so the example stays inside raylib's game loop.

Hooks generated from the schema are implemented in `main.c`:

- `OnFrameTimer` advances waves when the arena is clear.
- `OnPlayerHealthChanged` moves the session to game-over.
- `OnWaveChanged` moves the session to victory.
- `OnScoreChanged` opens an upgrade state at score thresholds.

The raylib code handles input and drawing; authoritative game data lives in `KekStateStore` slots generated from the schema.
