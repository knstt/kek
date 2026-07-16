# Remaining Open Findings

This file consolidates the unresolved findings that were previously split across `doc/example-design-findings.md` and `examples/warehouse/findings.md`.

## Generator And Schema

- The schema can describe state shape and hook dependency metadata, but state-to-state behavior is still handwritten C.
- The schema does not support arrays, enums, nested structs, optional fields, or references.
- The schema cannot express board cells, walls, packages, delivery zones, or other collection-like data directly.
- Cross-state invariants cannot be expressed, such as worker position inside map bounds or package delivery requiring the worker to carry it.
- Stream-backed `StandardInput` and `StandardOutput` still require manual stream registration and manual stream-to-state bridge code.
- The editor helps shape JSON schemas, but it cannot assist with handwritten C behavior, which remains most of the work in an example.

## Runtime And Events

- Multi-state updates are not transactional. A gameplay action that updates several states can expose intermediate states or leave partial changes if a later update fails.
- Each state slot update publishes its own `KEK_EVENT_STATE_CHANGED` event, so hooks can observe intermediate state unless the app deliberately uses one state as the final trigger.
- Resetting an app publishes several state-change events and relies on hook choices rather than explicit transaction semantics.
- Hook bodies read current state from `KekStateStore`, not a snapshot of the state that triggered the event. Queued events can therefore observe newer state than the event version implies.
- State-change events include type, slot, and version, but not a changed-state snapshot.
- The runtime does not expose event-version state values directly through hook contexts.
- Hook dependencies are descriptive only. Nothing prevents a hook from writing a state not listed in `writes`.
- Hook cycles are not detected or prevented. A hook triggered by `Worker.changed` can write `Worker` again and create a feedback loop unless the app guards it.
- The runtime has no first-class internal/silent write mode for hook-owned updates.
- The runtime has no timer or synthetic event source, so examples are naturally input-driven unless a custom runtime state is written.
- The runtime still does not provide hook-level event coalescing, render throttling, or scheduling backpressure for bursty terminal output.

## Example Application Code

- Stream-to-state bridging is still duplicated between examples.
- Text buffer ownership for generated `String` fields is still manual. Applications must provide stable buffers or string literals for borrowed `KekString` views.
- State-copy update helpers and reset helpers are still duplicated between examples.
- Game rules involving multiple states are handwritten and manually validated with generated `*_check()` functions.
- Multi-state commit ordering is manual. The warehouse example commits `Worker`, then `Package`, then `GameStatus` so rendering happens after dependent updates.
- The warehouse command flow still dispatches pending events immediately after publishing each `PlayerCommand` update as an application-level workaround for missing event snapshots.
- The warehouse `Worker` feedback-loop guard still records an ignored version and skips the matching follow-up event, which is fragile.
- Non-interactive smoke testing works partly because raw mode is a no-op for piped stdin and `q` exits immediately.

## Build And Project Structure

- Example Makefiles may benefit from a shared include file or a root build target that knows about all examples.
- There are still two generated-state storage APIs, `KekStateStorage` and `KekStateStore`. `KekStateStore` is the primary example-facing API, but the older single-state API remains visible in docs and code.

## Open Design Questions

- Should arrays and enums be added before building richer examples?
- Should the runtime support transactional batches across several generated state slots?
- Should hook descriptors define transaction boundaries or final render triggers?
- Should `reads` and `writes` be enforced, or remain documentation and future scheduling metadata?
- Should state-change events include the changed state snapshot, or should hook contexts expose the event version's value directly?
- Should the runtime detect or prevent hook cycles such as `Worker.changed -> write Worker`?
- Should hooks support event coalescing, backpressure, or render throttling for terminal-style output?
- Should stream input/output become declarative generated states, or stay manual runtime stream wiring?
- Should there be a first-class command/event state pattern instead of modeling commands as normal persisted state?
- Should example Makefiles share a common include, or should the root build know about all examples?
