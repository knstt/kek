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


def parse_states(source: str) -> list[State]:
    lines = source.splitlines()
    states: list[State] = []
    current: State | None = None
    section = "fields"
    depth = 0

    for line_number, raw_line in enumerate(lines, start=1):
        line = clean_line(raw_line)
        if not line:
            continue

        state_match = re.fullmatch(r"state\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{?", line)
        if state_match:
            if current is not None:
                raise SyntaxError(f"line {line_number}: nested state declarations are not supported")
            current = State(state_match.group(1))
            section = "fields"
            depth = 1 if line.endswith("{") else 0
            if depth == 0:
                raise SyntaxError(f"line {line_number}: expected '{{' after state name")
            continue

        if current is None:
            raise SyntaxError(f"line {line_number}: expected state declaration")

        if line == "}":
            depth -= 1
            if depth == 0:
                states.append(current)
                current = None
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
            current.fields.append(Field(field_match.group(1), field_match.group(2)))
            continue

        if section == "default":
            default_match = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_]*)\s*[:=]\s*(.+)", line)
            if not default_match:
                raise SyntaxError(f"line {line_number}: expected default assignment")
            current.defaults[default_match.group(1)] = default_match.group(2).strip()
            continue

        if section == "verify":
            current.verify_rules.append(line)
            continue

    if current is not None:
        raise SyntaxError(f"unterminated state {current.name}")
    return states


def c_type(type_name: str) -> str:
    return TYPE_MAP.get(type_name, type_name)


def generated_guard(name: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9]", "_", name).upper()
    return f"GENERATED_{cleaned}_H"


def field_map(state: State) -> dict[str, Field]:
    return {item.name: item for item in state.fields}


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


def emit_header(states: list[State], name: str) -> str:
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
        "typedef struct KekString {",
        "    const char* data;",
        "    size_t len;",
        "} KekString;",
        "",
        "KekString kek_string_from_cstr(const char* text);",
        "size_t kek_string_len(const KekString* text);",
        "",
    ]

    for state in states:
        lines.append(f"typedef struct {state.name} {{")
        for item in state.fields:
            lines.append(f"    {c_type(item.type_name)} {item.name};")
        lines.append(f"}} {state.name};")
        lines.append("")
        lines.append(f"{state.name} {state.name}_default(void);")
        lines.append(f"int {state.name}_verify(const {state.name}* state);")
        lines.append("")

    lines.extend([f"#endif /* {guard} */", ""])
    return "\n".join(lines)


def emit_source(states: list[State], name: str) -> str:
    header_name = f"{name}.h"
    lines = [
        "/* Generated by tools/generate_states.py. Do not edit manually. */",
        "#include <assert.h>",
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

        lines.append(f"int {state.name}_verify(const {state.name}* state) {{")
        lines.append("    assert(state != 0);")
        for rule in state.verify_rules:
            lines.append(f"    assert({translate_verify_rule(state, rule)});")
        lines.append("    return 1;")
        lines.append("}")
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
        states = parse_states(source)
        if not states:
            raise SyntaxError("no state declarations found")

        os.makedirs(args.out_dir, exist_ok=True)
        header_path = os.path.join(args.out_dir, f"{base_name}.h")
        source_path = os.path.join(args.out_dir, f"{base_name}.c")
        with open(header_path, "w", encoding="utf-8") as header_file:
            header_file.write(emit_header(states, base_name))
        with open(source_path, "w", encoding="utf-8") as c_file:
            c_file.write(emit_source(states, base_name))
    except (SyntaxError, ValueError) as error:
        print(f"generate_states.py: {error}", file=sys.stderr)
        return 1

    print(f"generated {header_path} and {source_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
