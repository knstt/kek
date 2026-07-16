# Generator Requirements

Related documents:

- [Generator Specification](specification.md)
- [Generator Architecture](architecture.md)
- [Runtime Requirements](../runtime/requirements.md)

## Scope

The generator converts a restricted JSON state schema into plain C header and source files.

It is responsible for data shape generation, default constructors, and invariant verification helpers. It is not responsible for runtime event handling or application behavior.

## Functional Requirements

| ID | Requirement | Current Status | Evidence |
| --- | --- | --- | --- |
| GEN-FR-001 | Parse JSON files containing state declarations. | Implemented | `tools/generate_states.py` |
| GEN-FR-002 | Parse typed fields with `name`, `type`, `default`, and optional `min`/`max`. | Implemented | `parse_document()` |
| GEN-FR-003 | Reject duplicate fields in one state. | Implemented | `parse_document()` |
| GEN-FR-004 | Support per-field default values. | Implemented | `Field.default` |
| GEN-FR-005 | Support per-field `min`/`max` constraints. | Implemented | `emit_field_checks()` |
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
| GEN-FR-020 | Generate a Markdown Mermaid graph of states and hook dependencies. | Implemented | `<name>.graph.md` |
| GEN-FR-021 | Generate additional per-state constructors as partial overrides of the default constructor. | Implemented | `Constructor` |
| GEN-FR-022 | Parse enum declarations and generate matching C enum typedefs. | Implemented | `Enum`, `emit_header()` |
| GEN-FR-023 | Support fixed-size field arrays with full-array defaults and per-element checks. | Implemented | `Field.array_length`, `emit_source()` |

## Non-Functional Requirements

| ID | Requirement | Current Status | Evidence |
| --- | --- | --- | --- |
| GEN-NFR-001 | Emit plain C ABI files. | Implemented | generated `.h` and `.c` |
| GEN-NFR-002 | Keep generated structs and helpers plain C while exposing explicit runtime metadata. | Implemented | generated data structs plus `KekStateDescriptor` table |
| GEN-NFR-003 | Keep generated string representation minimal. | Implemented | `KekString` contains `const char*` and `size_t` |
| GEN-NFR-004 | Fail generation with a diagnostic on syntax or validation errors. | Implemented | `except (SyntaxError, ValueError)` |

## Out Of Scope

- JSON Schema validation.
- Full language parsing.
- Function body compilation.
- Transition body compilation.
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
    Graph[Mermaid Graph Markdown]

    Schema --> Parser
    Parser --> Model
    Model --> Header
    Model --> Source
    Model --> Graph
```
