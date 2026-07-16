#!/usr/bin/env python3
import argparse
import os
import sys

from kekgen import (
    Constructor,
    Enum,
    Field,
    GeneratedFiles,
    Hook,
    Instance,
    State,
    STANDARD_STATES,
    emit_graph,
    emit_header,
    emit_source,
    parse_document,
    parse_source,
    render_all,
    render_all_model,
    render_all_schema,
    standard_states_payload,
    write_generated_files,
)
from kekgen.naming import c_identifier_from_type, generated_guard, state_type_macro
from kekgen.render import c_type, translate_constraint_value, translate_default

__all__ = [
    "Constructor",
    "Enum",
    "Field",
    "GeneratedFiles",
    "Hook",
    "Instance",
    "State",
    "STANDARD_STATES",
    "c_identifier_from_type",
    "c_type",
    "emit_graph",
    "emit_header",
    "emit_source",
    "generated_guard",
    "parse_document",
    "parse_source",
    "render_all",
    "render_all_model",
    "render_all_schema",
    "state_type_macro",
    "standard_states_payload",
    "translate_constraint_value",
    "translate_default",
    "write_generated_files",
]


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
        header_path, source_path, graph_path = write_generated_files(source, args.out_dir, base_name)
    except (SyntaxError, ValueError) as error:
        print(f"generate_states.py: {error}", file=sys.stderr)
        return 1

    print(f"generated {header_path}, {source_path}, and {graph_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
