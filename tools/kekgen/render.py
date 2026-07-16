import json
from importlib import resources
from string import Template

from .model import Field, Hook, State
from .naming import aggregate_state_name, c_identifier_from_type, generated_guard, state_type_macro


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


def emit_header(states: list[State], hooks: list[Hook], name: str) -> str:
    return render_template(
        "generated.h.tpl",
        guard=generated_guard(name),
        state_type_enum=render_state_type_enum(states),
        state_declarations=render_state_declarations(states),
        aggregate_declarations=render_aggregate_declarations(states, name),
        hook_count=len(hooks),
        hook_declarations=render_hook_declarations(hooks),
    )


def emit_source(states: list[State], hooks: list[Hook], name: str) -> str:
    aggregate_name = aggregate_state_name(name)
    return render_template(
        "generated.c.tpl",
        header_name=f"{name}.h",
        state_definitions=render_state_definitions(states),
        aggregate_name=aggregate_name,
        aggregate_field_defaults=render_aggregate_field_defaults(states),
        aggregate_checks=render_aggregate_checks(states),
        descriptor_entries=render_descriptor_entries(states),
        hook_dependency_arrays=render_hook_dependency_arrays(hooks),
        hook_descriptor_table=render_hook_descriptor_table(hooks),
    )


def emit_graph(states: list[State], hooks: list[Hook], name: str) -> str:
    return render_template(
        "generated.graph.md.tpl",
        name=name,
        graph_body=render_graph_body(states, hooks),
    )


def render_template(template_name: str, **values: object) -> str:
    template = resources.files("kekgen.templates").joinpath(template_name).read_text(encoding="utf-8")
    return Template(template).substitute({key: str(value) for key, value in values.items()})


def c_type(type_name: str) -> str:
    return TYPE_MAP.get(type_name, type_name)


def field_map(state: State) -> dict[str, Field]:
    return {item.name: item for item in state.fields}


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


def render_state_type_enum(states: list[State]) -> str:
    lines = ["typedef enum KekGeneratedStateType {"]
    for index, state in enumerate(states):
        lines.append(f"    {state_type_macro(state.name)} = {index},")
    lines.append(f"    KEK_STATE_TYPE_COUNT = {len(states)}")
    lines.append("} KekGeneratedStateType;")
    return "\n".join(lines)


def render_state_declarations(states: list[State]) -> str:
    lines: list[str] = []
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
    return "\n".join(lines).rstrip()


def render_aggregate_declarations(states: list[State], name: str) -> str:
    aggregate_name = aggregate_state_name(name)
    lines = [f"typedef struct {aggregate_name} {{"]
    for state in states:
        lines.append(f"    {state.name} {c_identifier_from_type(state.name)};")
    lines.append(f"}} {aggregate_name};")
    lines.append("")
    lines.append(f"{aggregate_name} {aggregate_name}_default(void);")
    lines.append(f"int {aggregate_name}_check(const {aggregate_name}* state);")
    lines.append(f"int {aggregate_name}_reset({aggregate_name}* state);")
    return "\n".join(lines)


def render_hook_declarations(hooks: list[Hook]) -> str:
    return "\n".join(f"void {hook.name}(KekHookContext* context);" for hook in hooks)


def render_state_definitions(states: list[State]) -> str:
    blocks = [render_single_state_definitions(state) for state in states]
    return "\n\n".join(blocks)


def render_single_state_definitions(state: State) -> str:
    fields = field_map(state)
    lines = [
        f"{state.name} {state.name}_default(void) {{",
        f"    {state.name} state = {{0}};",
    ]
    for field_item in state.fields:
        lines.append(f"    state.{field_item.name} = {translate_default(field_item, field_item.default)};")
    lines.extend(["    return state;", "}", ""])

    for constructor in state.constructors:
        lines.append(f"{state.name} {state.name}_{constructor.name}(void) {{")
        lines.append(f"    {state.name} state = {state.name}_default();")
        for field_name, value in constructor.values.items():
            lines.append(f"    state.{field_name} = {translate_default(fields[field_name], value)};")
        lines.extend(["    return state;", "}", ""])

    lines.extend(
        [
            f"void {state.name}_default_into(void* state) {{",
            "    if (state == 0) {",
            "        return;",
            "    }",
            f"    *(({state.name}*)state) = {state.name}_default();",
            "}",
            "",
            f"int {state.name}_check(const {state.name}* state) {{",
            "    if (state == 0) {",
            "        return 0;",
            "    }",
        ]
    )
    lines.extend(render_field_checks(state))
    lines.extend(
        [
            "    return 1;",
            "}",
            "",
            f"int {state.name}_check_void(const void* state) {{",
            f"    return {state.name}_check((const {state.name}*)state);",
            "}",
            "",
            f"int {state.name}_reset({state.name}* state) {{",
            "    if (state == 0) {",
            "        return 0;",
            "    }",
            f"    *state = {state.name}_default();",
            f"    return {state.name}_check(state);",
            "}",
            "",
            f"int {state.name}_reset_void(void* state) {{",
            f"    return {state.name}_reset(({state.name}*)state);",
            "}",
        ]
    )
    return "\n".join(lines)


def render_field_checks(state: State) -> list[str]:
    lines: list[str] = []
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
    return lines


def render_aggregate_field_defaults(states: list[State]) -> str:
    return "\n".join(
        f"    state.{c_identifier_from_type(state.name)} = {state.name}_default();" for state in states
    )


def render_aggregate_checks(states: list[State]) -> str:
    lines: list[str] = []
    for state in states:
        lines.append(f"    if (!{state.name}_check(&state->{c_identifier_from_type(state.name)})) {{")
        lines.append("        return 0;")
        lines.append("    }")
    return "\n".join(lines)


def render_descriptor_entries(states: list[State]) -> str:
    lines: list[str] = []
    for state in states:
        lines.append("    {")
        lines.append(f"        .type_id = {state_type_macro(state.name)},")
        lines.append(f"        .name = \"{state.name}\",")
        lines.append(f"        .size = sizeof({state.name}),")
        lines.append(f"        .set_default = {state.name}_default_into,")
        lines.append(f"        .check = {state.name}_check_void,")
        lines.append(f"        .reset = {state.name}_reset_void,")
        lines.append("    },")
    return "\n".join(lines)


def render_hook_dependency_arrays(hooks: list[Hook]) -> str:
    lines: list[str] = []
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
    return "\n".join(lines).rstrip()


def render_hook_descriptor_table(hooks: list[Hook]) -> str:
    if not hooks:
        return "const KekHookDescriptor* KekGeneratedHookDescriptors = 0;"

    lines = ["static const KekHookDescriptor KekGeneratedHookDescriptorData[KEK_GENERATED_HOOK_COUNT] = {"]
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
    return "\n".join(lines)


def render_graph_body(states: list[State], hooks: list[Hook]) -> str:
    lines: list[str] = []
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
    return "\n".join(lines)
