# Example Design Findings

This note captures findings from reviewing the project and sketching a second example before implementing it.

## Proposed Example

A small terminal "Warehouse Picker" demo fits the current generator/runtime shape.

State types:

- `StandardInput`: input text buffer.
- `StandardOutput`: rendered output buffer.
- `Worker`: worker position, carried package flag, energy, and score.
- `WarehouseMap`: fixed map width and height.
- `Package`: package position and delivery flag.
- `DeliveryZone`: delivery location.
- `GameStatus`: turn, message, done flag, and win flag.

Hooks:

- `HandleInputChanged`: triggered by `StandardInput.changed`, reads input, writes gameplay states.
- `RenderAfterStatusChanged`: triggered by `GameStatus.changed`, reads gameplay states, writes `StandardOutput`.

Gameplay:

- Move with WASD or arrow keys.
- Pick up a package by stepping onto it.
- Deliver it at the delivery zone.
- Energy decreases per move.
- The game ends when the package is delivered or energy reaches zero.

## Findings Before Implementation

- The root `Makefile` is hard-coded to `example/game.json`, `example/main.c`, generated name `game`, and output `main`, so additional examples are not first-class.
- The schema can describe state shape and hook dependency metadata, but all behavior still has to be handwritten in C.
- Hook `reads` and `writes` are metadata only. The runtime does not enforce them.
- Multi-state updates are not transactional. A gameplay action that updates several states can leave partial changes if a later update fails.
- Each state slot update publishes its own `KEK_EVENT_STATE_CHANGED` event, so hooks can observe intermediate state unless the application deliberately uses one state as the final trigger.
- Multiple state instances are supported by `KekStateStore`, but the schema has no way to declare instances.
- The schema does not support arrays, enums, nested structs, optional fields, or references.
- Cross-state invariants cannot be expressed, such as "worker position must be inside the map" or "a package cannot be delivered unless the worker was carrying it".
- `String` is a borrowed view, so applications must manually own stable buffers for any dynamic text.
- Generated enum names are mechanically uppercased, for example `StandardInput` becomes `KEK_STATE_TYPE_STANDARDINPUT`, which is less readable than `KEK_STATE_TYPE_STANDARD_INPUT`.
- Slot wiring is verbose. Each state needs a field in the app struct, a `kek_state_store_add()` call, a descriptor lookup, invalid-id checks, and usually typed accessor helpers.
- There are two generated-state storage APIs, `KekStateStorage` and `KekStateStore`. The newer slot-based store is more useful for examples, but the older API remains visible in docs and code.
- Runtime stream events must be manually bridged into generated input state before generated hooks can react to input.
- The editor helps shape JSON schemas, but it cannot assist with the handwritten C behavior, which is currently most of the work in an example.

## Open Questions

- Should examples live under `examples/<name>` with independent schemas, generated output directories, and Makefiles?
- Should the schema declare state instances in addition to state types?
- Should the generator emit typed store accessors and slot registration helpers?
- Should the runtime support transactional batches across several generated state slots?
- Should `reads` and `writes` be enforced, or remain documentation and future scheduling metadata?
- Should stream input/output become declarative generated states, or stay as manual runtime stream wiring?
- Should arrays and enums be added before building richer examples?
