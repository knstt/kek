# Generator Requirements

Related documents:

- [Generator Specification](specification.md)
- [Generator Architecture](architecture.md)
- [Runtime Requirements](../runtime/requirements.md)

## Scope

The generator converts a restricted `.kek` state schema into plain C header and source files.

It is responsible for data shape generation, default constructors, and invariant verification helpers. It is not responsible for runtime event handling or application behavior.

## Functional Requirements

| ID | Requirement | Current Status | Evidence |
| --- | --- | --- | --- |
| GEN-FR-001 | Parse `.kek` files containing `state` declarations. | Implemented | `tools/generate_states.py` |
| GEN-FR-002 | Parse typed fields written as `name: Type`. | Implemented | `parse_states()` |
| GEN-FR-003 | Reject duplicate fields in one state. | Implemented | `parse_states()` |
| GEN-FR-004 | Support `default` blocks for initial state values. | Implemented | `State.defaults` |
| GEN-FR-005 | Support `verify` blocks for simple invariants. | Implemented | `State.verify_rules` |
| GEN-FR-006 | Generate C structs for each state. | Implemented | `emit_header()` |
| GEN-FR-007 | Generate one default constructor per state. | Implemented | `emit_source()` |
| GEN-FR-008 | Generate one non-aborting check function per state. | Implemented | `emit_source()` |
| GEN-FR-009 | Generate an aggregate state struct for all parsed states. | Implemented | `aggregate_state_name()` |
| GEN-FR-010 | Generate aggregate default and check functions. | Implemented | `emit_source()` |
| GEN-FR-011 | Map known schema primitive types to C types. | Implemented | `TYPE_MAP` |
| GEN-FR-012 | Preserve unknown type names in generated C. | Implemented | `c_type()` |
| GEN-FR-013 | Provide a command-line interface for input path, output directory, and output base name. | Implemented | `argparse` in `main()` |
| GEN-FR-014 | Generate reset functions that restore default values. | Implemented | `emit_source()` |
| GEN-FR-015 | Generate state type identifiers. | Implemented | `KekGeneratedStateType` |
| GEN-FR-016 | Generate runtime-facing state descriptors. | Implemented | `KekGeneratedStateDescriptors` |
| GEN-FR-017 | Generate void-pointer descriptor adapters. | Implemented | `*_default_into()`, `*_check_void()`, `*_reset_void()` |
| GEN-FR-018 | Parse hook declarations with state-change triggers and read/write dependencies. | Implemented | `parse_source()` |
| GEN-FR-019 | Generate hook descriptor metadata. | Implemented | `KekGeneratedHookDescriptors` |

## Non-Functional Requirements

| ID | Requirement | Current Status | Evidence |
| --- | --- | --- | --- |
| GEN-NFR-001 | Emit plain C ABI files. | Implemented | generated `.h` and `.c` |
| GEN-NFR-002 | Keep generated structs and helpers plain C while exposing explicit runtime metadata. | Implemented | generated data structs plus `KekStateDescriptor` table |
| GEN-NFR-003 | Keep generated string representation minimal. | Implemented | `KekString` contains `const char*` and `size_t` |
| GEN-NFR-004 | Fail generation with a diagnostic on syntax or validation errors. | Implemented | `except (SyntaxError, ValueError)` |

## Out Of Scope

- Full language parsing.
- Function body compilation.
- Transition body compilation.
- Hook body compilation.
- Hook body compilation.
- Rollback semantics.
- String ownership and allocation policy.

## Requirement Relationship

```mermaid
flowchart LR
    Schema[Schema File]
    Parser[Parser]
    Model[State Model]
    Header[C Header]
    Source[C Source]

    Schema --> Parser
    Parser --> Model
    Model --> Header
    Model --> Source
```
