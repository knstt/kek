# Generator Architecture

Related documents:

- [Generator Requirements](requirements.md)
- [Generator Specification](specification.md)
- [Runtime Architecture](../runtime/architecture.md)

## Overview

The generator is a single Python script that performs parsing, validation, in-memory modeling, and C emission for states and hook metadata.

```mermaid
flowchart TB
    Input[Input Schema]
    Cleaner[clean_line and strip_comment]
    Parser[parse_states]
    StateModel[State and Field dataclasses]
    TypeMap[TYPE_MAP]
    VerifyTranslator[translate_verify_rule]
    DefaultTranslator[translate_default]
    HeaderEmitter[emit_header]
    SourceEmitter[emit_source]
    Header[Generated Header]
    Source[Generated Source]

    Input --> Cleaner
    Cleaner --> Parser
    Parser --> StateModel
    StateModel --> TypeMap
    StateModel --> VerifyTranslator
    StateModel --> DefaultTranslator
    StateModel --> HeaderEmitter
    StateModel --> SourceEmitter
    HeaderEmitter --> Header
    SourceEmitter --> Source
```

## Internal Model

The generator uses two dataclasses:

| Type | Responsibility |
| --- | --- |
| `Field` | Stores field name and schema type name. |
| `State` | Stores state name, fields, defaults, and verification rules. |
| `Hook` | Stores hook trigger plus read/write state dependencies. |

This model is intentionally close to the schema structure. There is no full AST.

## Parsing Strategy

Parsing is line-oriented and stateful.

The parser tracks:

- Current state or no active state.
- Current section: fields, default block, or verify block.
- Current hook declaration.
- Brace depth for state/block termination.

This keeps the parser small but limits expression and nesting support.

## Emission Strategy

The header and source emitters render strings from the `State` model.

The header emitter owns declarations and aggregate type layout.

The source emitter owns helper implementations, default construction, verification code, void-pointer adapters, state descriptor tables, and hook descriptor tables.

## Naming Strategy

| Generated Name | Rule |
| --- | --- |
| Header guard | Uppercase output base name with non-alphanumeric characters replaced by `_`, prefixed by `GENERATED_`, suffixed by `_H`. |
| Aggregate state | Output base name with first character uppercased, suffixed by `State`. |
| Aggregate fields | State type names converted from CamelCase to snake_case. |
| State functions | `<StateName>_default`, `<StateName>_check`, and `<StateName>_reset`. |
| Descriptor adapters | `<StateName>_default_into`, `<StateName>_check_void`, and `<StateName>_reset_void`. |

## Dependency Direction

```mermaid
flowchart LR
    Generator[tools/generate_states.py]
    GeneratedHeader[Generated Header]
    GeneratedSource[Generated Source]
    Consumer[C Consumer]

    Generator --> GeneratedHeader
    Generator --> GeneratedSource
    GeneratedHeader --> Consumer
    GeneratedSource --> Consumer
```

Generated structs and state helpers remain plain C data ABI. Generated descriptor declarations explicitly depend on the runtime `KekStateDescriptor` type so runtime state slots can be registered without handwritten adapters.

## Design Constraints

- The parser is intentionally not a full language parser.
- Generated state data is plain C.
- Generated verification rules are exposed through non-aborting check functions.
- Generated strings are borrowed views.
- Generated code is expected to be replaceable as the language matures.
