#!/usr/bin/env python3
import argparse
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


@dataclass
class State:
    name: str
    fields: list[Field] = field(default_factory=list)
    defaults: dict[str, str] = field(default_factory=dict)
    verify_rules: list[str] = field(default_factory=list)


@dataclass
class Hook:
    name: str
    event_type: str = ""
    state_name: str = ""
    reads: list[str] = field(default_factory=list)
    writes: list[str] = field(default_factory=list)


def strip_comment(line: str) -> str:
    in_string = False
    escaped = False
    for index, char in enumerate(line):
        if escaped:
            escaped = False
            continue
        if char == "\\" and in_string:
            escaped = True
            continue
        if char == '"':
            in_string = not in_string
            continue
        if not in_string and char == "/" and index + 1 < len(line) and line[index + 1] == "/":
            return line[:index]
    return line


def clean_line(line: str) -> str:
    return strip_comment(line).strip().rstrip(",;").strip()


def parse_source(source: str) -> tuple[list[State], list[Hook]]:
    lines = source.splitlines()
    states: list[State] = []
    hooks: list[Hook] = []
    current: State | None = None
    current_hook: Hook | None = None
    section = "fields"
    depth = 0

    for line_number, raw_line in enumerate(lines, start=1):
        line = clean_line(raw_line)
        if not line:
            continue

        state_match = re.fullmatch(r"state\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{?", line)
        if state_match:
            if current is not None or current_hook is not None:
                raise SyntaxError(f"line {line_number}: nested declarations are not supported")
            current = State(state_match.group(1))
            section = "fields"
            depth = 1 if line.endswith("{") else 0
            if depth == 0:
                raise SyntaxError(f"line {line_number}: expected '{{' after state name")
            continue

        hook_match = re.fullmatch(r"hook\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{?", line)
        if hook_match:
            if current is not None or current_hook is not None:
                raise SyntaxError(f"line {line_number}: nested declarations are not supported")
            current_hook = Hook(hook_match.group(1))
            section = "hook"
            depth = 1 if line.endswith("{") else 0
            if depth == 0:
                raise SyntaxError(f"line {line_number}: expected '{{' after hook name")
            continue

        if current is None and current_hook is None:
            raise SyntaxError(f"line {line_number}: expected state or hook declaration")

        if line == "}":
            depth -= 1
            if depth == 0 and current is not None:
                states.append(current)
                current = None
                section = "fields"
            elif depth == 0 and current_hook is not None:
                hooks.append(current_hook)
                current_hook = None
                section = "fields"
            elif depth == 1:
                section = "fields"
            elif depth < 0:
                raise SyntaxError(f"line {line_number}: unmatched '}}'")
            continue

        if line in ("default {", "verify {"):
            if depth != 1:
                raise SyntaxError(f"line {line_number}: nested blocks are not supported")
            section = "default" if line.startswith("default") else "verify"
            depth += 1
            continue

        if section == "fields":
            field_match = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_]*)\s*:\s*([A-Za-z_][A-Za-z0-9_]*)", line)
            if not field_match:
                raise SyntaxError(f"line {line_number}: expected field declaration")
            field_name = field_match.group(1)
            if any(item.name == field_name for item in current.fields):
                raise SyntaxError(f"line {line_number}: duplicate field {field_name}")
            current.fields.append(Field(field_match.group(1), field_match.group(2)))
            continue

        if section == "default":
            default_match = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_]*)\s*[:=]\s*(.+)", line)
            if not default_match:
                raise SyntaxError(f"line {line_number}: expected default assignment")
            field_name = default_match.group(1)
            if field_name in current.defaults:
                raise SyntaxError(f"line {line_number}: duplicate default for {field_name}")
            current.defaults[field_name] = default_match.group(2).strip()
            continue

        if section == "verify":
            current.verify_rules.append(line)
            continue

        if section == "hook":
            on_match = re.fullmatch(r"on\s+([A-Za-z_][A-Za-z0-9_]*)\.(changed)", line)
            if on_match:
                current_hook.state_name = on_match.group(1)
                current_hook.event_type = "KEK_EVENT_STATE_CHANGED"
                continue

            reads_match = re.fullmatch(r"reads\s+(.+)", line)
            if reads_match:
                current_hook.reads = parse_name_list(reads_match.group(1))
                continue

            writes_match = re.fullmatch(r"writes\s+(.+)", line)
            if writes_match:
                current_hook.writes = parse_name_list(writes_match.group(1))
                continue

            raise SyntaxError(f"line {line_number}: expected hook clause")

    if current is not None:
        raise SyntaxError(f"unterminated state {current.name}")
    if current_hook is not None:
        raise SyntaxError(f"unterminated hook {current_hook.name}")
    return states, hooks


def parse_states(source: str) -> list[State]:
    states, _hooks = parse_source(source)
    return states


def parse_name_list(value: str) -> list[str]:
    names = [item.strip() for item in value.split(",")]
    for name in names:
        if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
            raise SyntaxError(f"invalid name {name}")
    return names


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


def translate_default(field_item: Field, value: str) -> str:
    if field_item.type_name == "String":
        return f"kek_string_from_cstr({value})"
    if field_item.type_name == "bool":
        if value == "true":
            return "true"
        if value == "false":
            return "false"
    return value


def translate_verify_rule(state: State, rule: str) -> str:
    fields = field_map(state)
    output = rule

    for name in re.findall(r"\b([A-Za-z_][A-Za-z0-9_]*)\.len\(\)", rule):
        if name not in fields:
            raise ValueError(f"{state.name}: verify references unknown field {name}")
        if fields[name].type_name != "String":
            raise ValueError(f"{state.name}: len() is only supported for String field {name}")

    for item in sorted(state.fields, key=lambda value: len(value.name), reverse=True):
        if item.type_name == "String":
            output = re.sub(
                rf"\b{re.escape(item.name)}\.len\(\)",
                f"kek_string_len(&state->{item.name})",
                output,
            )

    def replace_name(match: re.Match[str]) -> str:
        start = match.start()
        if start >= 7 and output[start - 7:start] == "state->":
            return match.group(0)
        name = match.group(0)
        if name in fields:
            return f"state->{name}"
        if name in {"true", "false"}:
            return name
        return name

    return re.sub(r"\b[A-Za-z_][A-Za-z0-9_]*\b", replace_name, output)


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
        for field_name, value in state.defaults.items():
            if field_name not in fields:
                raise ValueError(f"{state.name}: default for unknown field {field_name}")
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
        for rule in state.verify_rules:
            lines.append(f"    if (!({translate_verify_rule(state, rule)})) {{")
            lines.append("        return 0;")
            lines.append("    }")
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

def main() -> int:
    parser = argparse.ArgumentParser(description="Generate C structs from Kek state schema files.")
    parser.add_argument("input", help="input .kek schema file")
    parser.add_argument("--out-dir", default="generated", help="output directory")
    parser.add_argument("--name", default=None, help="base name for generated .h/.c files")
    args = parser.parse_args()

    base_name = args.name or os.path.splitext(os.path.basename(args.input))[0]
    with open(args.input, "r", encoding="utf-8") as source_file:
        source = source_file.read()

    try:
        states, hooks = parse_source(source)
        if not states:
            raise SyntaxError("no state declarations found")
        validate_hooks(states, hooks)

        os.makedirs(args.out_dir, exist_ok=True)
        header_path = os.path.join(args.out_dir, f"{base_name}.h")
        source_path = os.path.join(args.out_dir, f"{base_name}.c")
        with open(header_path, "w", encoding="utf-8") as header_file:
            header_file.write(emit_header(states, hooks, base_name))
        with open(source_path, "w", encoding="utf-8") as c_file:
            c_file.write(emit_source(states, hooks, base_name))
    except (SyntaxError, ValueError) as error:
        print(f"generate_states.py: {error}", file=sys.stderr)
        return 1

    print(f"generated {header_path} and {source_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
