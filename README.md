# Kek State Schema Prototype

This repository prototypes two related components:

- A JSON state-schema generator that emits plain C data structures and verification helpers.
- A small single-threaded C runtime with event dispatch, hook dispatch, runtime state registration, and stream state support.
- A local no-framework browser editor for project-folder JSON schemas.

The current goal is to keep generated state data and runtime behavior separate. The generator owns the C data ABI. The runtime owns event processing and runtime-managed state callbacks.

## Documentation

Generator documentation:

- [Generator Requirements](doc/generator/requirements.md)
- [Generator Specification](doc/generator/specification.md)
- [Generator Architecture](doc/generator/architecture.md)

Runtime documentation:

- [Runtime Requirements](doc/runtime/requirements.md)
- [Runtime Specification](doc/runtime/specification.md)
- [Runtime Architecture](doc/runtime/architecture.md)

```mermaid
flowchart TD
    Readme[README.md]

    GenReq[doc/generator/requirements.md]
    GenSpec[doc/generator/specification.md]
    GenArch[doc/generator/architecture.md]

    RunReq[doc/runtime/requirements.md]
    RunSpec[doc/runtime/specification.md]
    RunArch[doc/runtime/architecture.md]

    Readme --> GenReq
    Readme --> GenSpec
    Readme --> GenArch
    Readme --> RunReq
    Readme --> RunSpec
    Readme --> RunArch

    GenReq --> GenSpec
    GenSpec --> GenArch

    RunReq --> RunSpec
    RunSpec --> RunArch
```

## Repository Layout

| Path | Purpose |
| --- | --- |
| `tools/generate_states.py` | JSON schema to C generator. |
| `tools/kek_editor.py` | Local HTTP server for the browser editor. |
| `editor/` | Plain HTML/CSS/JS project schema editor. |
| `runtime/` | C runtime modules. |
| `generated/` | Generated C output. |
| `Makefile` | Generation, compile-check, runtime build, and cleanup targets. |
| `doc/generator/` | Generator documentation. |
| `doc/runtime/` | Runtime documentation. |

## Build Commands

| Command | Purpose |
| --- | --- |
| `make generate` | Run the schema generator. |
| `make check` | Generate and compile generated C. |
| `make runtime` | Build the runtime executable. |
| `make editor` | Run the browser editor for the `example/` project folder. |
| `make all` | Run `check` and build the runtime executable. |
| `make clean` | Remove build/runtime artifacts. |

## Browser Editor

Run `make editor` and open `http://127.0.0.1:8080/`.

The editor serves the `example/` project folder. It loads JSON schema files from that
folder, saves schema changes back into that folder, and writes generated files to
`generated/`.

The editor is plain HTML, CSS, and JavaScript. It uses `tools/kek_editor.py` as a
small local API server and reuses the existing generator parser for validation.

## Runtime Example

`example/main.c` builds a small terminal dungeon demo around the generated `GameState`.
It registers stdin, stdout, and log streams with the runtime, stores generated
state through `KekStateStorage`, validates updates before swapping them in, and
renders the current state after game actions.

## Current Boundaries

Implemented in the generator:

- `state` declarations.
- Typed fields.
- Per-field defaults.
- Per-field `min`/`max` constraints emitted as non-aborting `*_check()` functions.
- Extra constructors as partial overrides of default constructors.
- Generated `*_reset()` functions that restore defaults.
- C header/source generation.

Implemented in the runtime:

- Bounded event queue.
- Synchronous subscriber dispatch.
- Fixed-size runtime state registry.
- `select()`-based event loop.
- Runtime stream states for file descriptors.
- Rollback-safe generated state storage.
- Independent generated state slots.
- Multiple instances per generated state type.
- Generated hook descriptors.

Not implemented yet:

- Full language parsing.
- Compiled function, transition, or hook bodies.
- Full generated runtime wiring.
- Rollback semantics.
- Per-state queues.
- Generated-state ownership management.
- Hook body compilation.
- Hook transaction enforcement.
