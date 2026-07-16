#!/usr/bin/env python3
import argparse
import json
import os
import re
import sys
from dataclasses import dataclass, field


TYPE_MAP = {
    "String": "KekString",
    "bool": "bool",
    "i32": "int32_t",
    "i64": "int64_t",
    "u32": "uint32_t",
    "u64": "uint64_t",
    "f32": "float",
    "f64": "double",
}


@dataclass
class Field:
    name: str
    type_name: str
    default: object
    minimum: object | None = None
    maximum: object | None = None


@dataclass
class State:
    name: str
    fields: list[Field] = field(default_factory=list)
    constructors: list["Constructor"] = field(default_factory=list)


@dataclass
class Constructor:
    name: str
    values: dict[str, object] = field(default_factory=dict)


@dataclass
class Hook:
    name: str
    event_type: str = ""
    state_name: str = ""
    reads: list[str] = field(default_factory=list)
    writes: list[str] = field(default_factory=list)


def require_identifier(value: object, label: str) -> str:
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", value):
        raise ValueError(f"{label} must be an identifier")
    return value


def parse_source(source: str) -> tuple[list[State], list[Hook]]:
    try:
        document = json.loads(source)
    except json.JSONDecodeError as error:
        raise SyntaxError(f"invalid JSON: {error.msg} at line {error.lineno}") from error
    return parse_document(document)


def parse_document(document: object) -> tuple[list[State], list[Hook]]:
    if not isinstance(document, dict):
        raise ValueError("schema root must be an object")

    state_items = document.get("states")
    if not isinstance(state_items, list) or not state_items:
        raise ValueError("schema must contain one or more states")

    states: list[State] = []
    state_names: set[str] = set()
    for state_index, state_item in enumerate(state_items):
        if not isinstance(state_item, dict):
            raise ValueError(f"states[{state_index}] must be an object")
        state_name = require_identifier(state_item.get("name"), f"states[{state_index}].name")
        if state_name in state_names:
            raise ValueError(f"duplicate state {state_name}")
        state_names.add(state_name)

        field_items = state_item.get("fields")
        if not isinstance(field_items, list) or not field_items:
            raise ValueError(f"{state_name}: state must contain one or more fields")

        fields: list[Field] = []
        field_names: set[str] = set()
        for field_index, field_item in enumerate(field_items):
            if not isinstance(field_item, dict):
                raise ValueError(f"{state_name}.fields[{field_index}] must be an object")
            field_name = require_identifier(field_item.get("name"), f"{state_name}.fields[{field_index}].name")
            if field_name in field_names:
                raise ValueError(f"{state_name}: duplicate field {field_name}")
            if "default" not in field_item:
                raise ValueError(f"{state_name}.{field_name}: field must declare default")
            field_names.add(field_name)
            fields.append(
                Field(
                    field_name,
                    require_identifier(field_item.get("type"), f"{state_name}.{field_name}.type"),
                    field_item.get("default"),
                    field_item.get("min"),
                    field_item.get("max"),
                )
            )

        constructors: list[Constructor] = []
        constructor_names: set[str] = set()
        for constructor_index, constructor_item in enumerate(state_item.get("constructors", [])):
            if not isinstance(constructor_item, dict):
                raise ValueError(f"{state_name}.constructors[{constructor_index}] must be an object")
            constructor_name = require_identifier(constructor_item.get("name"), f"{state_name}.constructors[{constructor_index}].name")
            if constructor_name == "default":
                raise ValueError(f"{state_name}: constructor name default is reserved")
            if constructor_name in constructor_names:
                raise ValueError(f"{state_name}: duplicate constructor {constructor_name}")
            values = constructor_item.get("values", {})
            if not isinstance(values, dict):
                raise ValueError(f"{state_name}.{constructor_name}: values must be an object")
            for field_name in values:
                if field_name not in field_names:
                    raise ValueError(f"{state_name}.{constructor_name}: value for unknown field {field_name}")
            constructor_names.add(constructor_name)
            constructors.append(Constructor(constructor_name, values))

        states.append(State(state_name, fields, constructors))

    hooks: list[Hook] = []
    hook_names: set[str] = set()
    for hook_index, hook_item in enumerate(document.get("hooks", [])):
        if not isinstance(hook_item, dict):
            raise ValueError(f"hooks[{hook_index}] must be an object")
        hook_name = require_identifier(hook_item.get("name"), f"hooks[{hook_index}].name")
        if hook_name in hook_names:
            raise ValueError(f"duplicate hook {hook_name}")
        on_item = hook_item.get("on", {})
        if not isinstance(on_item, dict):
            raise ValueError(f"{hook_name}.on must be an object")
        if on_item.get("event") != "changed":
            raise ValueError(f"{hook_name}: only changed events are supported")
        hook = Hook(
            hook_name,
            "KEK_EVENT_STATE_CHANGED",
            require_identifier(on_item.get("state"), f"{hook_name}.on.state"),
            parse_json_name_list(hook_item.get("reads", []), f"{hook_name}.reads"),
            parse_json_name_list(hook_item.get("writes", []), f"{hook_name}.writes"),
        )
        hook_names.add(hook_name)
        hooks.append(hook)

    validate_hooks(states, hooks)
    return states, hooks


def parse_json_name_list(value: object, label: str) -> list[str]:
    if not isinstance(value, list):
        raise ValueError(f"{label} must be an array")
    return [require_identifier(item, f"{label}[]") for item in value]


def c_type(type_name: str) -> str:
    return TYPE_MAP.get(type_name, type_name)


def generated_guard(name: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9]", "_", name).upper()
    return f"GENERATED_{cleaned}_H"


def c_identifier_from_type(name: str) -> str:
    output = re.sub(r"(.)([A-Z][a-z]+)", r"\1_\2", name)
    output = re.sub(r"([a-z0-9])([A-Z])", r"\1_\2", output)
    return output.lower()


def aggregate_state_name(name: str) -> str:
    return f"{name[:1].upper()}{name[1:]}State"


def state_type_macro(name: str) -> str:
    return f"KEK_STATE_TYPE_{name.upper()}"


def field_map(state: State) -> dict[str, Field]:
    return {item.name: item for item in state.fields}


def validate_hooks(states: list[State], hooks: list[Hook]) -> None:
    state_names = {state.name for state in states}
    for hook in hooks:
        if not hook.event_type or not hook.state_name:
            raise ValueError(f"{hook.name}: hook must declare an on clause")
        if hook.state_name not in state_names:
            raise ValueError(f"{hook.name}: hook references unknown state {hook.state_name}")
        for name in hook.reads + hook.writes:
            if name not in state_names:
                raise ValueError(f"{hook.name}: hook references unknown state {name}")


def c_string_literal(value: str) -> str:
    return json.dumps(value)


def translate_default(field_item: Field, value: object) -> str:
    if field_item.type_name == "String":
        if not isinstance(value, str):
            raise ValueError(f"{field_item.name}: String value must be a string")
        return f"kek_string_from_cstr({c_string_literal(value)})"
    if field_item.type_name == "bool":
        if value is True:
            return "true"
        if value is False:
            return "false"
        raise ValueError(f"{field_item.name}: bool value must be true or false")
    if isinstance(value, bool):
        raise ValueError(f"{field_item.name}: numeric value must not be boolean")
    if isinstance(value, (int, float)):
        return repr(value)
    if isinstance(value, str):
        return value
    raise ValueError(f"{field_item.name}: unsupported default value")


def translate_constraint_value(field_item: Field, value: object) -> str:
    if field_item.type_name == "String":
        if not isinstance(value, int) or isinstance(value, bool):
            raise ValueError(f"{field_item.name}: String min/max must be integer lengths")
        return str(value)
    return translate_default(field_item, value)


def emit_field_checks(state: State, lines: list[str]) -> None:
    for item in state.fields:
        if item.minimum is not None:
            lhs = f"kek_string_len(&state->{item.name})" if item.type_name == "String" else f"state->{item.name}"
            lines.append(f"    if ({lhs} < {translate_constraint_value(item, item.minimum)}) {{")
            lines.append("        return 0;")
            lines.append("    }")
        if item.maximum is not None:
            lhs = f"kek_string_len(&state->{item.name})" if item.type_name == "String" else f"state->{item.name}"
            lines.append(f"    if ({lhs} > {translate_constraint_value(item, item.maximum)}) {{")
            lines.append("        return 0;")
            lines.append("    }")


def emit_header(states: list[State], hooks: list[Hook], name: str) -> str:
    guard = generated_guard(name)
    lines = [
        "/* Generated by tools/generate_states.py. Do not edit manually. */",
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "#include <stdbool.h>",
        "#include <stddef.h>",
        "#include <stdint.h>",
        "",
        "#include \"../runtime/state_storage.h\"",
        "#include \"../runtime/hook.h\"",
        "",
        "typedef struct KekString {",
        "    const char* data;",
        "    size_t len;",
        "} KekString;",
        "",
        "KekString kek_string_from_cstr(const char* text);",
        "size_t kek_string_len(const KekString* text);",
        "",
    ]

    lines.append("typedef enum KekGeneratedStateType {")
    for index, state in enumerate(states):
        lines.append(f"    {state_type_macro(state.name)} = {index},")
    lines.append(f"    KEK_STATE_TYPE_COUNT = {len(states)}")
    lines.append("} KekGeneratedStateType;")
    lines.append("")

    for state in states:
        lines.append(f"typedef struct {state.name} {{")
        for item in state.fields:
            lines.append(f"    {c_type(item.type_name)} {item.name};")
        lines.append(f"}} {state.name};")
        lines.append("")
        lines.append(f"{state.name} {state.name}_default(void);")
        for constructor in state.constructors:
            lines.append(f"{state.name} {state.name}_{constructor.name}(void);")
        lines.append(f"void {state.name}_default_into(void* state);")
        lines.append(f"int {state.name}_check(const {state.name}* state);")
        lines.append(f"int {state.name}_check_void(const void* state);")
        lines.append(f"int {state.name}_reset({state.name}* state);")
        lines.append(f"int {state.name}_reset_void(void* state);")
        lines.append("")

    aggregate_name = aggregate_state_name(name)
    lines.append(f"typedef struct {aggregate_name} {{")
    for state in states:
        lines.append(f"    {state.name} {c_identifier_from_type(state.name)};")
    lines.append(f"}} {aggregate_name};")
    lines.append("")
    lines.append(f"{aggregate_name} {aggregate_name}_default(void);")
    lines.append(f"int {aggregate_name}_check(const {aggregate_name}* state);")
    lines.append(f"int {aggregate_name}_reset({aggregate_name}* state);")
    lines.append("")
    lines.append("extern const KekStateDescriptor KekGeneratedStateDescriptors[KEK_STATE_TYPE_COUNT];")
    lines.append("const KekStateDescriptor* kek_generated_state_descriptor(size_t type_id);")
    lines.append("")
    lines.append(f"#define KEK_GENERATED_HOOK_COUNT {len(hooks)}")
    for hook in hooks:
        lines.append(f"void {hook.name}(KekHookContext* context);")
    lines.append("extern const KekHookDescriptor* KekGeneratedHookDescriptors;")
    lines.append("")

    lines.extend([f"#endif /* {guard} */", ""])
    return "\n".join(lines)


def emit_source(states: list[State], hooks: list[Hook], name: str) -> str:
    header_name = f"{name}.h"
    lines = [
        "/* Generated by tools/generate_states.py. Do not edit manually. */",
        "#include <string.h>",
        f"#include \"{header_name}\"",
        "",
        "KekString kek_string_from_cstr(const char* text) {",
        "    KekString value;",
        "    value.data = text;",
        "    value.len = text ? strlen(text) : 0;",
        "    return value;",
        "}",
        "",
        "size_t kek_string_len(const KekString* text) {",
        "    return text ? text->len : 0;",
        "}",
        "",
    ]

    for state in states:
        fields = field_map(state)
        lines.append(f"{state.name} {state.name}_default(void) {{")
        lines.append(f"    {state.name} state = {{0}};")
        for field_item in state.fields:
            lines.append(f"    state.{field_item.name} = {translate_default(field_item, field_item.default)};")
        lines.append("    return state;")
        lines.append("}")
        lines.append("")

        for constructor in state.constructors:
            lines.append(f"{state.name} {state.name}_{constructor.name}(void) {{")
            lines.append(f"    {state.name} state = {state.name}_default();")
            for field_name, value in constructor.values.items():
                lines.append(f"    state.{field_name} = {translate_default(fields[field_name], value)};")
            lines.append("    return state;")
            lines.append("}")
            lines.append("")

        lines.append(f"void {state.name}_default_into(void* state) {{")
        lines.append("    if (state == 0) {")
        lines.append("        return;")
        lines.append("    }")
        lines.append(f"    *(({state.name}*)state) = {state.name}_default();")
        lines.append("}")
        lines.append("")

        lines.append(f"int {state.name}_check(const {state.name}* state) {{")
        lines.append("    if (state == 0) {")
        lines.append("        return 0;")
        lines.append("    }")
        emit_field_checks(state, lines)
        lines.append("    return 1;")
        lines.append("}")
        lines.append("")

        lines.append(f"int {state.name}_check_void(const void* state) {{")
        lines.append(f"    return {state.name}_check((const {state.name}*)state);")
        lines.append("}")
        lines.append("")

        lines.append(f"int {state.name}_reset({state.name}* state) {{")
        lines.append("    if (state == 0) {")
        lines.append("        return 0;")
        lines.append("    }")
        lines.append(f"    *state = {state.name}_default();")
        lines.append(f"    return {state.name}_check(state);")
        lines.append("}")
        lines.append("")

        lines.append(f"int {state.name}_reset_void(void* state) {{")
        lines.append(f"    return {state.name}_reset(({state.name}*)state);")
        lines.append("}")
        lines.append("")

    aggregate_name = aggregate_state_name(name)
    lines.append(f"{aggregate_name} {aggregate_name}_default(void) {{")
    lines.append(f"    {aggregate_name} state = {{0}};")
    for state in states:
        lines.append(f"    state.{c_identifier_from_type(state.name)} = {state.name}_default();")
    lines.append("    return state;")
    lines.append("}")
    lines.append("")

    lines.append(f"int {aggregate_name}_check(const {aggregate_name}* state) {{")
    lines.append("    if (state == 0) {")
    lines.append("        return 0;")
    lines.append("    }")
    for state in states:
        lines.append(f"    if (!{state.name}_check(&state->{c_identifier_from_type(state.name)})) {{")
        lines.append("        return 0;")
        lines.append("    }")
    lines.append("    return 1;")
    lines.append("}")
    lines.append("")

    lines.append(f"int {aggregate_name}_reset({aggregate_name}* state) {{")
    lines.append("    if (state == 0) {")
    lines.append("        return 0;")
    lines.append("    }")
    lines.append(f"    *state = {aggregate_name}_default();")
    lines.append(f"    return {aggregate_name}_check(state);")
    lines.append("}")
    lines.append("")

    lines.append("const KekStateDescriptor KekGeneratedStateDescriptors[KEK_STATE_TYPE_COUNT] = {")
    for state in states:
        lines.append("    {")
        lines.append(f"        .type_id = {state_type_macro(state.name)},")
        lines.append(f"        .name = \"{state.name}\",")
        lines.append(f"        .size = sizeof({state.name}),")
        lines.append(f"        .set_default = {state.name}_default_into,")
        lines.append(f"        .check = {state.name}_check_void,")
        lines.append(f"        .reset = {state.name}_reset_void,")
        lines.append("    },")
    lines.append("};")
    lines.append("")

    lines.append("const KekStateDescriptor* kek_generated_state_descriptor(size_t type_id) {")
    lines.append("    if (type_id >= KEK_STATE_TYPE_COUNT) {")
    lines.append("        return 0;")
    lines.append("    }")
    lines.append("    return &KekGeneratedStateDescriptors[type_id];")
    lines.append("}")
    lines.append("")

    for hook in hooks:
        if hook.reads:
            lines.append(f"static const size_t {hook.name}_reads[] = {{")
            for state_name in hook.reads:
                lines.append(f"    {state_type_macro(state_name)},")
            lines.append("};")
            lines.append("")
        if hook.writes:
            lines.append(f"static const size_t {hook.name}_writes[] = {{")
            for state_name in hook.writes:
                lines.append(f"    {state_type_macro(state_name)},")
            lines.append("};")
            lines.append("")

    if hooks:
        lines.append("static const KekHookDescriptor KekGeneratedHookDescriptorData[KEK_GENERATED_HOOK_COUNT] = {")
        for hook in hooks:
            reads_name = f"{hook.name}_reads" if hook.reads else "0"
            writes_name = f"{hook.name}_writes" if hook.writes else "0"
            lines.append("    {")
            lines.append(f"        .name = \"{hook.name}\",")
            lines.append(f"        .event_type = {hook.event_type},")
            lines.append(f"        .state_type_id = {state_type_macro(hook.state_name)},")
            lines.append(f"        .reads = {reads_name},")
            lines.append(f"        .read_count = {len(hook.reads)},")
            lines.append(f"        .writes = {writes_name},")
            lines.append(f"        .write_count = {len(hook.writes)},")
            lines.append(f"        .run = {hook.name},")
            lines.append("    },")
        lines.append("};")
        lines.append("const KekHookDescriptor* KekGeneratedHookDescriptors = KekGeneratedHookDescriptorData;")
    else:
        lines.append("const KekHookDescriptor* KekGeneratedHookDescriptors = 0;")
    lines.append("")

    return "\n".join(lines)


def emit_graph(states: list[State], hooks: list[Hook], name: str) -> str:
    lines = [
        "<!-- Generated by tools/generate_states.py. Do not edit manually. -->",
        f"# {name} State Graph",
        "",
        "```mermaid",
        "flowchart LR",
    ]

    for state in states:
        lines.append(f"    S_{state.name}[\"{state.name}\"]")

    for hook in hooks:
        lines.append(f"    H_{hook.name}{{\"{hook.name}\"}}")

    for hook in hooks:
        lines.append(f"    S_{hook.state_name} -->|changed| H_{hook.name}")
        for state_name in hook.reads:
            lines.append(f"    S_{state_name} -. reads .-> H_{hook.name}")
        for state_name in hook.writes:
            lines.append(f"    H_{hook.name} -->|writes| S_{state_name}")

    lines.extend(["```", ""])
    return "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate C structs from Kek JSON schema files.")
    parser.add_argument("input", help="input JSON schema file")
    parser.add_argument("--out-dir", default="generated", help="output directory")
    parser.add_argument("--name", default=None, help="base name for generated .h/.c files")
    args = parser.parse_args()

    base_name = args.name or os.path.splitext(os.path.basename(args.input))[0]
    with open(args.input, "r", encoding="utf-8") as source_file:
        source = source_file.read()

    try:
        states, hooks = parse_source(source)

        os.makedirs(args.out_dir, exist_ok=True)
        header_path = os.path.join(args.out_dir, f"{base_name}.h")
        source_path = os.path.join(args.out_dir, f"{base_name}.c")
        graph_path = os.path.join(args.out_dir, f"{base_name}.graph.md")
        with open(header_path, "w", encoding="utf-8") as header_file:
            header_file.write(emit_header(states, hooks, base_name))
        with open(source_path, "w", encoding="utf-8") as c_file:
            c_file.write(emit_source(states, hooks, base_name))
        with open(graph_path, "w", encoding="utf-8") as graph_file:
            graph_file.write(emit_graph(states, hooks, base_name))
    except (SyntaxError, ValueError) as error:
        print(f"generate_states.py: {error}", file=sys.stderr)
        return 1

    print(f"generated {header_path}, {source_path}, and {graph_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
