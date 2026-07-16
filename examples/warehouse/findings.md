# Warehouse Example Findings

This note captures additional findings from implementing `examples/warehouse`.

## What Was Added

- `warehouse.json`: schema for a small terminal warehouse delivery game with a chained hook graph.
- `main.c`: handwritten runtime application using generated warehouse state types and hooks.
- `Makefile`: independent build entry point for the warehouse example.
- Generated files are emitted as `../../generated/warehouse.h`, `../../generated/warehouse.c`, and `../../generated/warehouse.graph.md`.

Current hook chain:

- `HandleInputChanged`: converts appended input bytes into `PlayerCommand` updates.
- `ApplyCommandChanged`: reacts to `PlayerCommand`, moves `Worker`, resets state, or quits.
- `UpdatePackageAfterWorkerChanged`: reacts to `Worker`, picks up or drops the package, and updates `GameStatus`.
- `UpdateStatusAfterPackageChanged`: reacts to `Package`, finalizes win/loss state.
- `RenderAfterStatusChanged`: reacts to `GameStatus` and renders `StandardOutput`.

## Verification

Commands run from `examples/warehouse`:

```sh
make all
printf "q" | ./warehouse
printf "ddddddssssdq" | ./warehouse
```

The build and short quit smoke test succeeded. The longer movement smoke test exercised the hook cascade and completed, but it also exposed stdout stream buffer pressure because many full-screen renders were queued from one piped input burst.

## Complications Encountered During Implementation

- Generated headers currently include runtime headers using paths like `../runtime/state_storage.h`. That works when output is root-level `generated/`, but fails if generated files are placed inside `examples/warehouse/generated/`.
- Because of that include path assumption, the example can have its own Makefile but not fully self-contained generated output without changing the generator.
- The generated header name is fixed by the generator output base name, so the C app must include `generated/warehouse.h` from the repository root rather than a local generated header.
- A second example can easily overwrite or coexist in the shared root `generated/` directory, but there is no namespacing beyond the output base name.
- Adding one example duplicated a large amount of app code from `example/main.c`: stream-to-state bridging, text buffer ownership, state slot registration, typed accessors, state-copy updates, reset helpers, and hook registry setup.
- Slot registration is especially repetitive and error-prone. Every state needs a descriptor lookup, slot field, invalid-id check, and later typed accessor.
- The app still needs to know generated enum spelling such as `KEK_STATE_TYPE_WAREHOUSEMAP` and `KEK_STATE_TYPE_GAMESTATUS`.
- The generated schema cannot express board cells, walls, packages, or delivery zones as collections. The example uses scalar fields and fixed map dimensions instead.
- Game rules that involve multiple states are entirely handwritten and manually validated with `Worker_check()`, `Package_check()`, and `GameStatus_check()`.
- Multi-state commit ordering had to be chosen manually. The app commits `Worker`, then `Package`, then `GameStatus` so rendering is triggered after dependent states are updated.
- Resetting the game publishes several state-change events, but only the final `GameStatus` reset triggers rendering. This relies on hook choices rather than explicit transaction semantics.
- Input events still require a manual bridge from `KEK_EVENT_STREAM_DATA` to a generated `StandardInput` state update before `HandleInputChanged` can run.
- Output state exists mostly as metadata/history. Writing to the terminal still uses `KekStream` directly and then mirrors written bytes into `StandardOutput`.
- `String` fields require stable app-owned buffers or string literals. This is manageable for messages and accumulated input/output, but it is easy to misuse.
- The runtime has no timer or synthetic event source, so examples are naturally input-driven unless a custom runtime state is written.
- Non-interactive smoke testing works only because raw mode is a no-op for piped stdin and `q` quits immediately.
- Splitting behavior across hooks made ordering more visible. The app now relies on `PlayerCommand -> Worker -> Package -> GameStatus -> StandardOutput` event order.
- Hook bodies read current state from `KekStateStore`, not a snapshot of the state that triggered the event. When several `PlayerCommand` updates were queued before dispatch, every queued command event observed the latest command value instead of the command value at that event version.
- The example works around that by dispatching pending events immediately after publishing each `PlayerCommand` update. This is an application-level workaround, not a runtime guarantee.
- A hook triggered by `Worker` also needs to write `Worker` when the worker picks up or drops a package. That creates a natural feedback-loop risk because the runtime has no cycle detection or "internal write" suppression.
- The example avoids the `Worker` feedback loop by recording the worker version it just wrote and ignoring the matching follow-up event. This is fragile and easy to get wrong.
- Rendering after every status update can overflow the bounded stdout stream buffer when input is piped quickly. The runtime reports dropped bytes rather than applying backpressure to state updates or hooks.
- More hooks made the graph more useful, but also made it clear that hook dependencies are descriptive only. Nothing prevents a hook from writing a state not listed in `writes`.
- State-to-state behavior is still entirely in handwritten C. The schema can show the dependency graph, but it cannot express the transformation from one state to another.

## Design Questions Raised By Implementation

- Should the generator accept an include-prefix option so generated output can live inside an example folder?
- Should generated headers include runtime headers as `runtime/...` instead of `../runtime/...` and rely on include paths?
- Should example Makefiles share a common include, or should the root build know about all examples?
- Should the generator produce typed state-store helpers such as `warehouse_store_add_defaults()` or `warehouse_worker()`?
- Should hook descriptors be able to define a final render trigger or grouped transaction boundary?
- Should state updates support batching so multi-state gameplay actions are atomic and publish one consolidated event?
- Should stream-backed `StandardInput` and `StandardOutput` be generated or supplied by a reusable helper module?
- Should generated enum names use snake-case conversion to improve readability?
- Should the schema grow arrays/enums before more realistic examples are attempted?
- Should state-change events include the changed state snapshot, or should hook contexts expose the event version's value directly?
- Should the runtime detect or prevent hook cycles such as `Worker.changed -> write Worker`?
- Should hooks support event coalescing, backpressure, or render throttling for terminal-style output?
- Should there be a first-class command/event state pattern instead of modeling commands as normal persisted state?
