# Remaining Findings

This file consolidates the unresolved findings that were previously split across `doc/example-design-findings.md` and `examples/warehouse/findings.md`.

## Generator And Schema

- The schema can describe state shape and hook dependency metadata, but state-to-state behavior is still handwritten C.
- Cross-state invariants cannot be expressed, such as worker position inside map bounds or package delivery requiring the worker to carry it.
- Stream-backed `StandardInput` and `StandardOutput` still require manual stream registration, but reusable bridge helpers now cover stream-to-state text synchronization.
- The editor helps shape JSON schemas, but it cannot assist with handwritten C behavior, which remains most of the work in an example.

## Runtime And Events

- Multi-state updates now have an explicit `kek_state_store_update_many()` batch API, but transaction boundaries are still application-selected rather than inferred from hook descriptors.
- Each changed state slot in a successful batch still publishes its own `KEK_EVENT_STATE_CHANGED` event after commit, followed by one aggregate `KEK_EVENT_STATE_BATCH_CHANGED` event.
- Resetting declared generated slots now uses a generated transactional reset helper, but dynamic instance deletion/recreation is still application-specific.
- Hook bodies can read event-version state snapshots through `kek_hook_event_state()` when the changed state fits the fixed snapshot capacity; larger states still require current-store reads.
- Hook `writes` are enforced during hook execution, but `reads` are still descriptive only.
- Direct hook writes to the triggering state type are prevented, but longer hook cycles across multiple state types are not detected.
- The runtime has no first-class internal/silent write mode for hook-owned updates.
- The runtime still does not provide hook-level event coalescing, render throttling, or scheduling backpressure for bursty terminal output.

## Example Application Code

- Text buffer ownership for generated `String` fields is still manual. Applications must provide stable buffers or string literals for borrowed `KekString` views.
- State-copy update helpers are still duplicated between examples.
- Game rules involving multiple states are handwritten and manually validated with generated `*_check()` functions.
- Multi-state commit ordering is reduced by `kek_state_store_update_many()`, but examples still choose their own batch boundaries.
- The warehouse command flow still dispatches pending events immediately after publishing each `PlayerCommand` update as an application-level command-state pattern.
- Non-interactive smoke testing now exits on stdin EOF, but examples still do not have dedicated automated assertions.

## Build And Project Structure

- There are still two generated-state storage APIs, `KekStateStorage` and `KekStateStore`. `KekStateStore` is the primary example-facing API, but the older single-state API remains visible in docs and code.

## Open Design Questions

- Should hook descriptors define transaction boundaries or final render triggers?
- Should `reads` be enforced, or remain documentation and future scheduling metadata?
- Should the runtime detect longer hook cycles across multiple state types?
- Should hooks support event coalescing, backpressure, or render throttling for terminal-style output?
- Should stream input/output become declarative generated states, or stay manual runtime stream wiring?
- Should there be a first-class command/event state pattern instead of modeling commands as normal persisted state?
