# Kek State Schema Prototype

This repository is currently focused on a small bridge between the new language design and a plain C event runtime proof of concept.

The immediate goal is not to compile the full language. The first useful milestone is to describe data states in a small language file and generate plain C data structures from it. Runtime behavior, transitions, hooks, and application logic are implemented manually in C for now.

## Current Scope

Supported now:

- `state` declarations
- typed fields
- `default` blocks
- simple `verify` blocks
- C header/source generation
- `assert(...)` based verification

Not supported yet:

- compiled `fn` bodies
- compiled `transition` bodies
- compiled `on_state_change` hooks
- full expression parsing
- rollback semantics
- per-state queues
- generated runtime wiring

The current runtime proof of concept is single-threaded and event based. It provides standard runtime states such as I/O streams, while generated `game.kek` states remain plain data that application code updates and verifies.

## Runtime Direction

The runtime is plain C and split into small modules under `runtime/`:

- `runtime/event.*`: bounded event queue and synchronous subscriber dispatch.
- `runtime/state.h`: generic runtime state callbacks used by the event loop.
- `runtime/runtime.*`: single-threaded `select()` event loop and state registry.
- `runtime/stream.*`: standard stream state implementation for file descriptors.

Streams are runtime-provided states. A stream state registers file descriptor interests with the event loop, publishes stream events when data arrives, and flushes buffered writes when the descriptor is writable.

Application code bridges runtime states into generated states. The demo in `main.c` maps stdin stream events into `StandardInput`, tracks writes in `StandardOutput`, verifies both generated states, and keeps `Player` initialized from `game.kek`.

## Architecture Direction

The intended architecture has three layers.

1. State schema language

The `.kek` file describes state shapes and basic invariants:

```kek
state StandardInput {
  input: String

  default {
    input: ""
  }

  verify {
    input.len() <= 1000
  }
}
```

2. Generated C ABI

The generator emits C structs plus helper functions:

```c
typedef struct StandardInput {
    KekString input;
} StandardInput;

StandardInput StandardInput_default(void);
int StandardInput_verify(const StandardInput* state);
```

3. Handwritten C/C++ behavior

Transitions, hooks, runtime event handlers, and application logic are handwritten against the generated types:

```c
StandardInput AppendInput(StandardInput input, KekString text) {
    /* handwritten for now */
    return input;
}
```

Later, the compiler can gradually take over transition and hook generation without changing the generated C data ABI too much.

## Schema Syntax

### State Declaration

```kek
state Player {
  name: String
  health: i32

  default {
    name: "Player"
    health: 100
  }

  verify {
    name.len() <= 64
    health >= 0
    health <= 100
  }
}
```

Fields use `name: Type`. Commas and semicolons are optional line endings.

### Supported Types

The generator currently maps these language types to C:

| Language | C |
| --- | --- |
| `String` | `KekString` |
| `bool` | `bool` |
| `i32` | `int32_t` |
| `i64` | `int64_t` |
| `u32` | `uint32_t` |
| `u64` | `uint64_t` |
| `f32` | `float` |
| `f64` | `double` |

Unknown types are emitted as written, which lets handwritten C provide the matching declarations later.

### Defaults

Default values are copied into the generated constructor with minimal translation:

```kek
default {
  input: ""
  health: 100
}
```

Generates:

```c
StandardInput state = {0};
state.input = kek_string_from_cstr("");
state.health = 100;
return state;
```

Fields without defaults keep the zero-initialized value from `state = {0}`.

### Verification

Verification is intentionally simple. Every line in `verify` becomes an `assert(...)`.

```kek
verify {
  input.len() <= 1000
  health >= 0 && health <= 100
}
```

Generates:

```c
int Player_verify(const Player* state) {
    assert(kek_string_len(&state->input) <= 1000);
    assert(state->health >= 0 && state->health <= 100);
    return 1;
}
```

The translation is deliberately stupid:

- `field.len()` becomes `kek_string_len(&state->field)` for `String` fields.
- known field names become `state->field`.
- operators are copied directly.
- each non-empty rule becomes one `assert(...)`.

This is enough for basic invariants and keeps the first generator easy to replace later.

## Generated Runtime Types

The generated header currently includes a tiny `KekString` definition:

```c
typedef struct KekString {
    const char* data;
    size_t len;
} KekString;
```

This is intentionally minimal. Ownership and allocation are not solved in the schema generator yet. String defaults point at string literals.

## Commands

Generate C from `game.kek`:

```sh
make generate
```

Check that generated C compiles:

```sh
make check
```

Build the C runtime demo:

```sh
make runtime
```

Build both generated code and the runtime demo:

```sh
make all
```

Remove generated output:

```sh
make clean
```

Direct generator usage:

```sh
python3 tools/generate_states.py game.kek --out-dir generated --name game
```

## Current Files

- `game.kek`: state schema example used by the generator.
- `tools/generate_states.py`: Python schema-to-C generator.
- `generated/game.h`: generated C declarations after `make generate`.
- `generated/game.c`: generated C definitions after `make generate`.
- `main.c`: C runtime demo wiring standard stream states into generated game states.
- `runtime/`: single-threaded event runtime with generic state registration and standard stream states.

## Next Milestones

1. Keep state generation stable and small.
2. Add declarations for external C transitions/hooks without generating their bodies.
3. Let handwritten C wire generated states into standard runtime states.
4. Introduce generated transition wrappers that call handwritten C bodies and run verification before commit.
5. Add state-change events and hook dispatch once the data ABI is proven.
