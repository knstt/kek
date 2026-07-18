import json
from importlib import resources
from string import Template

from .model import Enum, Field, Hook, Instance, State
from .naming import c_identifier_from_type, generated_guard, state_type_macro


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


def emit_header(
    states: list[State],
    hooks: list[Hook],
    instances: list[Instance] | str | None = None,
    name: str | None = None,
    enums: list[Enum] | None = None,
) -> str:
    if name is None:
        name = str(instances)
        instances = []
    instances = instances if isinstance(instances, list) else []
    enums = enums or []
    return render_template(
        "generated.h.tpl",
        guard=generated_guard(name),
        enum_declarations=render_enum_declarations(enums),
        state_type_enum=render_state_type_enum(states),
        state_declarations=render_state_declarations(states),
        instance_declarations=render_instance_declarations(states, hooks, instances, name),
        hook_count=len(hooks),
        hook_declarations=render_hook_declarations(hooks),
    )


def emit_source(
    states: list[State],
    hooks: list[Hook],
    instances: list[Instance] | str | None = None,
    name: str | None = None,
    enums: list[Enum] | None = None,
) -> str:
    if name is None:
        name = str(instances)
        instances = []
    instances = instances if isinstance(instances, list) else []
    enums = enums or []
    return render_template(
        "generated.c.tpl",
        header_name=f"{name}.h",
        state_definitions=render_state_definitions(states, enums),
        descriptor_entries=render_descriptor_entries(states),
        instance_definitions=render_instance_definitions(states, hooks, instances, name, enums),
        hook_dependency_arrays=render_hook_dependency_arrays(hooks),
        hook_descriptor_table=render_hook_descriptor_table(hooks),
    )


def emit_graph(states: list[State], hooks: list[Hook], instances: list[Instance] | str | None = None, name: str | None = None) -> str:
    if name is None:
        name = str(instances)
        instances = []
    instances = instances if isinstance(instances, list) else []
    return render_template(
        "generated.graph.md.tpl",
        name=name,
        graph_body=render_graph_body(states, hooks, instances),
    )


def render_template(template_name: str, **values: object) -> str:
    template = resources.files("kekgen.templates").joinpath(template_name).read_text(encoding="utf-8")
    return Template(template).substitute({key: str(value) for key, value in values.items()})


def c_type(type_name: str) -> str:
    return TYPE_MAP.get(type_name, type_name)


def enum_value_name(enum: Enum, value: str) -> str:
    return f"{enum.name}_{value}"


def enum_map(enums: list[Enum]) -> dict[str, Enum]:
    return {item.name: item for item in enums}


def field_map(state: State) -> dict[str, Field]:
    return {item.name: item for item in state.fields}


def c_string_literal(value: str) -> str:
    return json.dumps(value)


def translate_default(field_item: Field, value: object, enums: dict[str, Enum] | None = None) -> str:
    enums = enums or {}
    enum = enums.get(field_item.type_name)
    if enum is not None:
        if not isinstance(value, str) or value not in enum.values:
            raise ValueError(f"{field_item.name}: enum value must be one of {enum.values}")
        return enum_value_name(enum, value)
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


def translate_constraint_value(field_item: Field, value: object, enums: dict[str, Enum] | None = None) -> str:
    if field_item.type_name == "String":
        if not isinstance(value, int) or isinstance(value, bool):
            raise ValueError(f"{field_item.name}: String min/max must be integer lengths")
        return str(value)
    return translate_default(field_item, value, enums)


def render_enum_declarations(enums: list[Enum]) -> str:
    lines: list[str] = []
    for enum in enums:
        lines.append(f"typedef enum {enum.name} {{")
        for index, value in enumerate(enum.values):
            lines.append(f"    {enum_value_name(enum, value)} = {index},")
        lines.append(f"}} {enum.name};")
        lines.append("")
    return "\n".join(lines).rstrip()


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
            if item.array_length is None:
                lines.append(f"    {c_type(item.type_name)} {item.name};")
            else:
                lines.append(f"    {c_type(item.type_name)} {item.name}[{item.array_length}];")
        lines.append(f"}} {state.name};")
        lines.append("")
        for index, item in enumerate(state.fields):
            lines.append(
                f"#define {state_type_macro(state.name)}_FIELD_{item.name.upper()} "
                f"(1ull << {index})"
            )
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


def generated_prefix(name: str) -> str:
    return c_identifier_from_type(name)


def instance_slots_name(name: str) -> str:
    return f"{name[:1].upper()}{name[1:]}StateSlots"


def runtime_binding_name(name: str) -> str:
    return f"{name[:1].upper()}{name[1:]}RuntimeBinding"


def runtime_wrapper_name(name: str) -> str:
    return f"{name[:1].upper()}{name[1:]}Runtime"


def runtime_buffers_name(name: str) -> str:
    return f"{name[:1].upper()}{name[1:]}RuntimeBuffers"


def update_item_name(name: str) -> str:
    return f"{name[:1].upper()}{name[1:]}UpdateItem"


def runtime_buffer_fields(states: list[State], instances: list[Instance]) -> list[tuple[Instance, Field]]:
    fields: list[tuple[Instance, Field]] = []
    state_by_name = {state.name: state for state in states}
    for instance in instances:
        state = state_by_name.get(instance.state_name)
        if state is None:
            continue
        for field in instance_string_buffer_fields(state):
            fields.append((instance, field))
    return fields


def render_instance_declarations(states: list[State], hooks: list[Hook], instances: list[Instance], name: str) -> str:
    prefix = generated_prefix(name)
    slots_name = instance_slots_name(name)
    lines: list[str] = [f"typedef struct {slots_name} {{"]
    if instances:
        for instance in instances:
            lines.append(f"    size_t {instance.name};")
    else:
        lines.append("    size_t unused;")
    lines.append(f"}} {slots_name};")
    lines.append("")
    item_name = update_item_name(name)
    lines.append(f"typedef KekStateStoreUpdateItem {item_name};")
    lines.append("")
    lines.append(f"void {prefix}_state_slots_init_invalid({slots_name}* slots);")
    binding_name = runtime_binding_name(name)
    lines.append(f"typedef struct {binding_name} {binding_name};")
    lines.append(f"int {prefix}_state_slots_add_declared(KekStateStore* store, {slots_name}* slots, {binding_name}* binding);")
    lines.append(f"int {prefix}_state_slots_remove_declared(KekStateStore* store, {slots_name}* slots);")
    lines.append(f"int {prefix}_state_slots_reset_declared(KekStateStore* store, const {slots_name}* slots);")
    lines.append("")
    buffers_name = runtime_buffers_name(name)
    lines.append(f"typedef struct {buffers_name} {{")
    buffer_fields = runtime_buffer_fields(states, instances)
    if buffer_fields:
        for instance, field in buffer_fields:
            lines.append(f"    char {instance.name}_{field.name}_buffer[{field.maximum + 1}];")
    else:
        lines.append("    size_t unused;")
    lines.append(f"}} {buffers_name};")
    lines.append("")
    lines.append(f"struct {binding_name} {{")
    lines.append("    KekRuntime* runtime;")
    lines.append("    KekStateStore state_store;")
    lines.append("    KekHookRegistry hook_registry;")
    if hooks:
        lines.append(f"    KekHookDescriptor hook_descriptors[{len(hooks)}];")
    lines.append(f"    {buffers_name} buffers;")
    lines.append(f"    {slots_name} slots;")
    lines.append("};")
    lines.append("")
    lines.append(f"int {prefix}_runtime_binding_init({binding_name}* binding, KekRuntime* runtime, void* app_context);")
    lines.append(f"void {prefix}_runtime_binding_destroy({binding_name}* binding);")
    lines.append("")
    runtime_name = runtime_wrapper_name(name)
    lines.append(f"typedef struct {runtime_name} {{")
    lines.append("    KekRuntimeApp app;")
    if hooks:
        lines.append(f"    KekHookDescriptor hook_descriptors[{len(hooks)}];")
    lines.append(f"    {buffers_name} buffers;")
    lines.append(f"    {slots_name} slots;")
    lines.append(f"}} {runtime_name};")
    lines.append("")
    lines.append(f"int {prefix}_runtime_init({runtime_name}* runtime, void* app_context);")
    lines.append(f"void {prefix}_runtime_destroy({runtime_name}* runtime);")
    lines.append(f"KekRuntime* {prefix}_get_runtime({runtime_name}* runtime);")
    lines.append(f"const KekRuntime* {prefix}_get_runtime_const(const {runtime_name}* runtime);")
    lines.append(f"KekStateStore* {prefix}_get_store({runtime_name}* runtime);")
    lines.append(f"const KekStateStore* {prefix}_get_store_const(const {runtime_name}* runtime);")
    lines.append(f"{slots_name}* {prefix}_get_slots({runtime_name}* runtime);")
    lines.append(f"const {slots_name}* {prefix}_get_slots_const(const {runtime_name}* runtime);")
    lines.append(f"int {prefix}_dispatch({runtime_name}* runtime);")
    lines.append(f"int {prefix}_update_many({runtime_name}* runtime, const {item_name}* updates, size_t update_count);")
    lines.append("")
    for state in states:
        snake = c_identifier_from_type(state.name)
        macro = state_type_macro(state.name)
        lines.append(f"size_t {prefix}_{snake}_create(KekStateStore* store);")
        lines.append(f"size_t {prefix}_{snake}_create_with(KekStateStore* store, const {state.name}* initial);")
        lines.append(f"int {prefix}_{snake}_delete(KekStateStore* store, size_t slot_id);")
        lines.append(f"{state.name}* {prefix}_{snake}_slot(KekStateStore* store, size_t slot_id);")
        lines.append(f"const {state.name}* {prefix}_{snake}_slot_const(const KekStateStore* store, size_t slot_id);")
        lines.append(f"size_t {prefix}_{snake}_first(const KekStateStore* store);")
        lines.append(f"size_t {prefix}_{snake}_next(const KekStateStore* store, size_t after_slot_id);")
        lines.append(f"size_t {prefix}_create_{snake}({runtime_name}* runtime);")
        lines.append(f"size_t {prefix}_create_{snake}_with({runtime_name}* runtime, const {state.name}* initial);")
        lines.append(f"int {prefix}_delete_{snake}({runtime_name}* runtime, size_t slot_id);")
        lines.append(f"{state.name}* {prefix}_{snake}_at({runtime_name}* runtime, size_t slot_id);")
        lines.append(f"const {state.name}* {prefix}_{snake}_at_const(const {runtime_name}* runtime, size_t slot_id);")
        lines.append(f"size_t {prefix}_first_{snake}(const {runtime_name}* runtime);")
        lines.append(f"size_t {prefix}_next_{snake}(const {runtime_name}* runtime, size_t after_slot_id);")
        lines.append(f"int {prefix}_count_{snake}(const {runtime_name}* runtime);")
        if any(field.name == "active" and field.type_name == "bool" and field.array_length is None for field in state.fields):
            lines.append(f"int {prefix}_count_active_{snake}(const {runtime_name}* runtime);")
        lines.append(f"int {prefix}_update_{snake}_slot({runtime_name}* runtime, size_t slot_id, KekStateStorageUpdateFn update, void* context, uint64_t changed_fields);")
        lines.append(f"{item_name} {prefix}_{snake}_slot_update_item(size_t slot_id, KekStateStorageUpdateFn update, void* context, uint64_t changed_fields);")
        for field in state.fields:
            if field.array_length is None and field.type_name != "String":
                lines.append(f"int {prefix}_{snake}_set_{field.name}(KekStateStore* store, size_t slot_id, {c_type(field.type_name)} value);")
                lines.append(f"int {prefix}_set_{snake}_slot_{field.name}({runtime_name}* runtime, size_t slot_id, {c_type(field.type_name)} value);")
        lines.append(f"#define {prefix.upper()}_{snake.upper()}_STATE_TYPE {macro}")
        lines.append("")
    declared_predicates: set[str] = set()
    for instance in instances:
        instance_state = next((item for item in states if item.name == instance.state_name), None)
        lines.append(f"{instance.state_name}* {prefix}_{instance.name}(KekStateStore* store, const {slots_name}* slots);")
        lines.append(
            f"const {instance.state_name}* {prefix}_{instance.name}_const(const KekStateStore* store, const {slots_name}* slots);"
        )
        lines.append(f"{instance.state_name}* {prefix}_{instance.name}_current({runtime_name}* runtime);")
        lines.append(f"const {instance.state_name}* {prefix}_{instance.name}_current_const(const {runtime_name}* runtime);")
        lines.append(f"size_t {prefix}_{instance.name}_slot_id(const {runtime_name}* runtime);")
        lines.append(f"int {prefix}_update_{instance.name}({runtime_name}* runtime, KekStateStorageUpdateFn update, void* context, uint64_t changed_fields);")
        lines.append(f"{item_name} {prefix}_{instance.name}_update_item({runtime_name}* runtime, KekStateStorageUpdateFn update, void* context, uint64_t changed_fields);")
        if instance_state is not None:
            instance_snake = c_identifier_from_type(instance.state_name)
            for field in instance_state.fields:
                if field.array_length is None and field.type_name != "String":
                    lines.append(f"int {prefix}_set_{instance.name}_{field.name}({runtime_name}* runtime, {c_type(field.type_name)} value);")
            if instance.name != instance_snake and instance_snake not in declared_predicates:
                lines.append(f"int {prefix}_is_declared_{instance_snake}_slot(const {runtime_name}* runtime, size_t slot_id);")
                declared_predicates.add(instance_snake)
        lines.append(f"#define {prefix.upper()}_{instance.name.upper()}_STATE_TYPE {state_type_macro(instance.state_name)}")
        lines.append("")
    for state in states:
        string_fields = [field for field in state.fields if field.type_name == "String" and field.array_length is None]
        if len(string_fields) == 1:
            snake = c_identifier_from_type(state.name)
            field_name = string_fields[0].name
            lines.append(
                f"int {prefix}_{snake}_set_{field_name}(KekStateStore* store, size_t slot_id, const char* data, size_t len);"
            )
    return "\n".join(lines).rstrip()


def render_hook_declarations(hooks: list[Hook]) -> str:
    return "\n".join(f"int {hook.name}(KekHookContext* context);" for hook in hooks)


def render_state_definitions(states: list[State], enums: list[Enum] | None = None) -> str:
    enum_by_name = enum_map(enums or [])
    blocks = [render_single_state_definitions(state, enum_by_name) for state in states]
    return "\n\n".join(blocks)


def render_single_state_definitions(state: State, enums: dict[str, Enum]) -> str:
    fields = field_map(state)
    lines = [
        f"{state.name} {state.name}_default(void) {{",
        f"    {state.name} state = {{0}};",
    ]
    for field_item in state.fields:
        lines.extend(render_field_default_assignment("state", field_item, field_item.default, enums))
    lines.extend(["    return state;", "}", ""])

    for constructor in state.constructors:
        lines.append(f"{state.name} {state.name}_{constructor.name}(void) {{")
        lines.append(f"    {state.name} state = {state.name}_default();")
        for field_name, value in constructor.values.items():
            lines.extend(render_field_default_assignment("state", fields[field_name], value, enums))
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
    lines.extend(render_field_checks(state, enums))
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


def render_field_default_assignment(target: str, field_item: Field, value: object, enums: dict[str, Enum]) -> list[str]:
    if field_item.array_length is None:
        return [f"    {target}.{field_item.name} = {translate_default(field_item, value, enums)};"]
    if not isinstance(value, list):
        raise ValueError(f"{field_item.name}: array default must be an array")
    if len(value) != field_item.array_length:
        raise ValueError(f"{field_item.name}: array default must contain {field_item.array_length} values")
    lines: list[str] = []
    scalar_field = Field(field_item.name, field_item.type_name, None, field_item.minimum, field_item.maximum)
    for index, item in enumerate(value):
        lines.append(f"    {target}.{field_item.name}[{index}] = {translate_default(scalar_field, item, enums)};")
    return lines


def instance_string_buffer_fields(state: State) -> list[Field]:
    return [
        field
        for field in state.fields
        if field.type_name == "String" and field.array_length is None and isinstance(field.maximum, int)
    ]


def state_field_by_name(state: State, field_name: str) -> Field:
    for field in state.fields:
        if field.name == field_name:
            return field
    raise ValueError(f"{state.name}: unknown field {field_name}")


def render_field_checks(state: State, enums: dict[str, Enum] | None = None) -> list[str]:
    lines: list[str] = []
    for item in state.fields:
        if item.array_length is None:
            lines.extend(render_single_value_checks(item, f"state->{item.name}", enums))
            continue
        for index in range(item.array_length):
            lines.extend(render_single_value_checks(item, f"state->{item.name}[{index}]", enums))
    return lines


def render_single_value_checks(field_item: Field, lhs: str, enums: dict[str, Enum] | None = None) -> list[str]:
    lines: list[str] = []
    check_lhs = f"kek_string_len(&{lhs})" if field_item.type_name == "String" else lhs
    if field_item.minimum is not None:
        lines.append(f"    if ({check_lhs} < {translate_constraint_value(field_item, field_item.minimum, enums)}) {{")
        lines.append("        return 0;")
        lines.append("    }")
    if field_item.maximum is not None:
        lines.append(f"    if ({check_lhs} > {translate_constraint_value(field_item, field_item.maximum, enums)}) {{")
        lines.append("        return 0;")
        lines.append("    }")
    return lines


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


def render_instance_definitions(states: list[State], hooks: list[Hook], instances: list[Instance], name: str, enums: list[Enum] | None = None) -> str:
    prefix = generated_prefix(name)
    slots_name = instance_slots_name(name)
    binding_name = runtime_binding_name(name)
    runtime_name = runtime_wrapper_name(name)
    buffers_name = runtime_buffers_name(name)
    item_name = update_item_name(name)
    state_by_name = {state.name: state for state in states}
    enum_by_name = enum_map(enums or [])
    lines: list[str] = [
        f"void {prefix}_state_slots_init_invalid({slots_name}* slots) {{",
        "    if (slots == 0) {",
        "        return;",
        "    }",
    ]
    if instances:
        for instance in instances:
            lines.append(f"    slots->{instance.name} = KEK_STATE_INVALID_ID;")
    else:
        lines.append("    slots->unused = KEK_STATE_INVALID_ID;")
    lines.extend(["}", ""])

    lines.extend(
        [
            f"static int {prefix}_state_slots_add_declared_with_buffers(KekStateStore* store, {slots_name}* slots, {buffers_name}* buffers) {{",
            "    if (store == 0 || slots == 0 || buffers == 0) {",
            "        return 0;",
            "    }",
            f"    {prefix}_state_slots_init_invalid(slots);",
        ]
    )
    for instance in instances:
        state = state_by_name[instance.state_name]
        initial_expr = "0"
        if instance.constructor_name is not None or instance.values or instance_string_buffer_fields(state):
            constructor = f"{state.name}_{instance.constructor_name}" if instance.constructor_name is not None else f"{state.name}_default"
            lines.append(f"    {state.name} {instance.name}_initial = {constructor}();")
            for field in instance_string_buffer_fields(state):
                lines.append(f"    {instance.name}_initial.{field.name} = (KekString){{buffers->{instance.name}_{field.name}_buffer, 0}};")
            for field_name, value in instance.values.items():
                lines.extend(render_field_default_assignment(f"{instance.name}_initial", state_field_by_name(state, field_name), value, enum_by_name))
            initial_expr = f"&{instance.name}_initial"
        lines.append(
            f"    slots->{instance.name} = kek_state_store_add(store, "
            f"&KekGeneratedStateDescriptors[{state_type_macro(instance.state_name)}], {initial_expr});"
        )
        lines.append(f"    if (slots->{instance.name} == KEK_STATE_INVALID_ID) {{")
        lines.append(f"        {prefix}_state_slots_remove_declared(store, slots);")
        lines.append("        return 0;")
        lines.append("    }")
    lines.extend(["    return 1;", "}", ""])

    lines.extend(
        [
            f"int {prefix}_state_slots_add_declared(KekStateStore* store, {slots_name}* slots, {binding_name}* binding) {{",
            "    if (binding == 0) {",
            "        return 0;",
            "    }",
            f"    return {prefix}_state_slots_add_declared_with_buffers(store, slots, &binding->buffers);",
            "}",
            "",
        ]
    )

    lines.extend(
        [
            f"int {prefix}_state_slots_remove_declared(KekStateStore* store, {slots_name}* slots) {{",
            "    if (store == 0 || slots == 0) {",
            "        return 0;",
            "    }",
            "    int ok = 1;",
        ]
    )
    for instance in instances:
        lines.append(f"    if (slots->{instance.name} != KEK_STATE_INVALID_ID) {{")
        lines.append(f"        ok = kek_state_store_remove(store, slots->{instance.name}) && ok;")
        lines.append(f"        slots->{instance.name} = KEK_STATE_INVALID_ID;")
        lines.append("    }")
    lines.extend(["    return ok;", "}", ""])

    lines.extend(
        [
            "static void kek_generated_reset_slot(void* draft, void* context) {",
            "    const KekStateDescriptor* descriptor = (const KekStateDescriptor*)context;",
            "    if (descriptor != 0 && descriptor->reset != 0) {",
            "        descriptor->reset(draft);",
            "    }",
            "}",
            "",
            f"int {prefix}_state_slots_reset_declared(KekStateStore* store, const {slots_name}* slots) {{",
            "    if (store == 0 || slots == 0) {",
            "        return 0;",
            "    }",
        ]
    )
    if instances:
        lines.append(f"    KekStateStoreUpdateItem updates[{len(instances)}];")
        lines.append(f"    const KekStateDescriptor* descriptors[{len(instances)}];")
        lines.append("    size_t count = 0;")
        for instance in instances:
            lines.append(f"    if (slots->{instance.name} != KEK_STATE_INVALID_ID) {{")
            lines.append(f"        descriptors[count] = kek_state_store_descriptor(store, slots->{instance.name});")
            lines.append("        if (descriptors[count] == 0) {")
            lines.append("            return 0;")
            lines.append("        }")
            lines.append(f"        updates[count] = (KekStateStoreUpdateItem){{slots->{instance.name}, kek_generated_reset_slot, (void*)descriptors[count], KEK_EVENT_CHANGED_FIELDS_UNKNOWN}};")
            lines.append("        count++;")
            lines.append("    }")
        lines.append("    return kek_state_store_update_many(store, updates, count);")
    else:
        lines.append("    (void)store;")
        lines.append("    return 1;")
    lines.extend(["}", ""])

    lines.extend(
        [
            f"int {prefix}_runtime_binding_init({binding_name}* binding, KekRuntime* runtime, void* app_context) {{",
            "    if (binding == 0 || runtime == 0) {",
            "        return 0;",
            "    }",
            "    memset(binding, 0, sizeof(*binding));",
            "    binding->runtime = runtime;",
            "    kek_state_store_init(&binding->state_store, runtime);",
            f"    if (!{prefix}_state_slots_add_declared(&binding->state_store, &binding->slots, binding)) {{",
            "        kek_state_store_destroy(&binding->state_store);",
            "        memset(binding, 0, sizeof(*binding));",
            "        return 0;",
            "    }",
            "    kek_hook_registry_init(&binding->hook_registry, runtime, &binding->state_store, app_context);",
        ]
    )
    if hooks:
        lines.append("    memcpy(binding->hook_descriptors, KekGeneratedHookDescriptors, sizeof(binding->hook_descriptors));")
        for index, hook in enumerate(hooks):
            if hook.instance_name is not None:
                lines.append(f"    binding->hook_descriptors[{index}].state_slot_id = binding->slots.{hook.instance_name};")
        lines.append("    if (!kek_hook_registry_add_many(&binding->hook_registry, binding->hook_descriptors, KEK_GENERATED_HOOK_COUNT)) {")
    else:
        lines.append("    if (!kek_hook_registry_add_many(&binding->hook_registry, KekGeneratedHookDescriptors, KEK_GENERATED_HOOK_COUNT)) {")
    lines.extend(
        [
            "        kek_state_store_destroy(&binding->state_store);",
            "        memset(binding, 0, sizeof(*binding));",
            "        return 0;",
            "    }",
            "    kek_hook_registry_attach(&binding->hook_registry);",
            "    return 1;",
            "}",
            "",
            f"void {prefix}_runtime_binding_destroy({binding_name}* binding) {{",
            "    if (binding == 0) {",
            "        return;",
            "    }",
            "    kek_hook_registry_detach(&binding->hook_registry);",
            "    kek_state_store_destroy(&binding->state_store);",
            "    memset(binding, 0, sizeof(*binding));",
            "}",
            "",
        ]
    )

    lines.extend(
        [
            f"int {prefix}_runtime_init({runtime_name}* runtime, void* app_context) {{",
            "    if (runtime == 0) {",
                "        return 0;",
            "    }",
            "    memset(runtime, 0, sizeof(*runtime));",
            "    if (!kek_runtime_app_init(&runtime->app, app_context)) {",
            "        memset(runtime, 0, sizeof(*runtime));",
            "        return 0;",
            "    }",
            f"    if (!{prefix}_state_slots_add_declared_with_buffers(kek_runtime_app_store(&runtime->app), &runtime->slots, &runtime->buffers)) {{",
            "        kek_runtime_app_destroy(&runtime->app);",
            "        memset(runtime, 0, sizeof(*runtime));",
            "        return 0;",
            "    }",
        ]
    )
    if hooks:
        lines.append("    memcpy(runtime->hook_descriptors, KekGeneratedHookDescriptors, sizeof(runtime->hook_descriptors));")
        for index, hook in enumerate(hooks):
            if hook.instance_name is not None:
                lines.append(f"    runtime->hook_descriptors[{index}].state_slot_id = runtime->slots.{hook.instance_name};")
        lines.append("    if (!kek_runtime_app_bind_hooks(&runtime->app, runtime->hook_descriptors, KEK_GENERATED_HOOK_COUNT)) {")
    else:
        lines.append("    if (!kek_runtime_app_bind_hooks(&runtime->app, KekGeneratedHookDescriptors, KEK_GENERATED_HOOK_COUNT)) {")
    lines.extend(
        [
            f"        {prefix}_state_slots_remove_declared(kek_runtime_app_store(&runtime->app), &runtime->slots);",
            "        kek_runtime_app_destroy(&runtime->app);",
            "        memset(runtime, 0, sizeof(*runtime));",
            "        return 0;",
            "    }",
            "    return 1;",
            "}",
            "",
            f"void {prefix}_runtime_destroy({runtime_name}* runtime) {{",
            "    if (runtime == 0) {",
            "        return;",
            "    }",
            "    kek_runtime_app_destroy(&runtime->app);",
            "    memset(runtime, 0, sizeof(*runtime));",
            "}",
            "",
            f"KekRuntime* {prefix}_get_runtime({runtime_name}* runtime) {{",
            "    return runtime ? kek_runtime_app_runtime(&runtime->app) : 0;",
            "}",
            "",
            f"const KekRuntime* {prefix}_get_runtime_const(const {runtime_name}* runtime) {{",
            "    return runtime ? kek_runtime_app_runtime_const(&runtime->app) : 0;",
            "}",
            "",
            f"KekStateStore* {prefix}_get_store({runtime_name}* runtime) {{",
            "    return runtime ? kek_runtime_app_store(&runtime->app) : 0;",
            "}",
            "",
            f"const KekStateStore* {prefix}_get_store_const(const {runtime_name}* runtime) {{",
            "    return runtime ? kek_runtime_app_store_const(&runtime->app) : 0;",
            "}",
            "",
            f"{slots_name}* {prefix}_get_slots({runtime_name}* runtime) {{",
            "    return runtime ? &runtime->slots : 0;",
            "}",
            "",
            f"const {slots_name}* {prefix}_get_slots_const(const {runtime_name}* runtime) {{",
            "    return runtime ? &runtime->slots : 0;",
            "}",
            "",
            f"int {prefix}_dispatch({runtime_name}* runtime) {{",
            "    if (runtime == 0) {",
            "        return 0;",
            "    }",
            "    return kek_runtime_app_dispatch(&runtime->app);",
            "}",
            "",
            f"int {prefix}_update_many({runtime_name}* runtime, const {item_name}* updates, size_t update_count) {{",
            f"    return kek_state_store_update_many({prefix}_get_store(runtime), updates, update_count);",
            "}",
            "",
        ]
    )

    for state in states:
        snake = c_identifier_from_type(state.name)
        macro = state_type_macro(state.name)
        lines.extend(
            [
                f"size_t {prefix}_{snake}_create(KekStateStore* store) {{",
                f"    return kek_state_store_add_default(store, &KekGeneratedStateDescriptors[{macro}]);",
                "}",
                "",
                f"size_t {prefix}_{snake}_create_with(KekStateStore* store, const {state.name}* initial) {{",
                f"    return kek_state_store_add(store, &KekGeneratedStateDescriptors[{macro}], initial);",
                "}",
                "",
                f"int {prefix}_{snake}_delete(KekStateStore* store, size_t slot_id) {{",
                f"    const KekStateDescriptor* descriptor = kek_state_store_descriptor(store, slot_id);",
                f"    if (descriptor == 0 || descriptor->type_id != {macro}) {{",
                "        return 0;",
                "    }",
                "    return kek_state_store_remove(store, slot_id);",
                "}",
                "",
                f"{state.name}* {prefix}_{snake}_slot(KekStateStore* store, size_t slot_id) {{",
                f"    const KekStateDescriptor* descriptor = kek_state_store_descriptor(store, slot_id);",
                f"    if (descriptor == 0 || descriptor->type_id != {macro}) {{",
                "        return 0;",
                "    }",
                f"    return ({state.name}*)kek_state_store_current(store, slot_id);",
                "}",
                "",
                f"const {state.name}* {prefix}_{snake}_slot_const(const KekStateStore* store, size_t slot_id) {{",
                f"    const KekStateDescriptor* descriptor = kek_state_store_descriptor(store, slot_id);",
                f"    if (descriptor == 0 || descriptor->type_id != {macro}) {{",
                "        return 0;",
                "    }",
                f"    return (const {state.name}*)kek_state_store_current_const(store, slot_id);",
                "}",
                "",
                f"size_t {prefix}_{snake}_first(const KekStateStore* store) {{",
                f"    return kek_state_store_find_first(store, {macro});",
                "}",
                "",
                f"size_t {prefix}_{snake}_next(const KekStateStore* store, size_t after_slot_id) {{",
                f"    return kek_state_store_find_next(store, {macro}, after_slot_id);",
                "}",
                "",
                f"size_t {prefix}_create_{snake}({runtime_name}* runtime) {{",
                f"    return {prefix}_{snake}_create({prefix}_get_store(runtime));",
                "}",
                "",
                f"size_t {prefix}_create_{snake}_with({runtime_name}* runtime, const {state.name}* initial) {{",
                f"    return {prefix}_{snake}_create_with({prefix}_get_store(runtime), initial);",
                "}",
                "",
                f"int {prefix}_delete_{snake}({runtime_name}* runtime, size_t slot_id) {{",
                f"    return {prefix}_{snake}_delete({prefix}_get_store(runtime), slot_id);",
                "}",
                "",
                f"{state.name}* {prefix}_{snake}_at({runtime_name}* runtime, size_t slot_id) {{",
                f"    return {prefix}_{snake}_slot({prefix}_get_store(runtime), slot_id);",
                "}",
                "",
                f"const {state.name}* {prefix}_{snake}_at_const(const {runtime_name}* runtime, size_t slot_id) {{",
                f"    return {prefix}_{snake}_slot_const({prefix}_get_store_const(runtime), slot_id);",
                "}",
                "",
                f"size_t {prefix}_first_{snake}(const {runtime_name}* runtime) {{",
                f"    return {prefix}_{snake}_first({prefix}_get_store_const(runtime));",
                "}",
                "",
                f"size_t {prefix}_next_{snake}(const {runtime_name}* runtime, size_t after_slot_id) {{",
                f"    return {prefix}_{snake}_next({prefix}_get_store_const(runtime), after_slot_id);",
                "}",
                "",
                f"int {prefix}_count_{snake}(const {runtime_name}* runtime) {{",
                "    int count = 0;",
                f"    for (size_t slot = {prefix}_first_{snake}(runtime); slot != KEK_STATE_INVALID_ID; slot = {prefix}_next_{snake}(runtime, slot)) {{",
                "        count++;",
                "    }",
                "    return count;",
                "}",
                "",
            ]
        )
        if any(field.name == "active" and field.type_name == "bool" and field.array_length is None for field in state.fields):
            lines.extend(
                [
                    f"int {prefix}_count_active_{snake}(const {runtime_name}* runtime) {{",
                    "    int count = 0;",
                    f"    for (size_t slot = {prefix}_first_{snake}(runtime); slot != KEK_STATE_INVALID_ID; slot = {prefix}_next_{snake}(runtime, slot)) {{",
                    f"        const {state.name}* state = {prefix}_{snake}_at_const(runtime, slot);",
                    "        if (state && state->active) {",
                    "            count++;",
                    "        }",
                    "    }",
                    "    return count;",
                    "}",
                    "",
                ]
            )
        lines.extend(
            [
                f"int {prefix}_update_{snake}_slot({runtime_name}* runtime, size_t slot_id, KekStateStorageUpdateFn update, void* context, uint64_t changed_fields) {{",
                f"    return kek_state_store_update_fields({prefix}_get_store(runtime), slot_id, update, context, changed_fields);",
                "}",
                "",
                f"{item_name} {prefix}_{snake}_slot_update_item(size_t slot_id, KekStateStorageUpdateFn update, void* context, uint64_t changed_fields) {{",
                f"    {item_name} item = {{slot_id, update, context, changed_fields}};",
                "    return item;",
                "}",
                "",
            ]
        )
        for field in state.fields:
            if field.array_length is not None or field.type_name == "String":
                continue
            lines.extend(
                [
                    f"typedef struct {state.name}_{field.name}_FieldUpdate {{",
                    f"    {c_type(field.type_name)} value;",
                    f"}} {state.name}_{field.name}_FieldUpdate;",
                    "",
                    f"static void {prefix}_{snake}_set_{field.name}_update(void* draft, void* context) {{",
                    f"    {state.name}* state = ({state.name}*)draft;",
                    f"    const {state.name}_{field.name}_FieldUpdate* update = (const {state.name}_{field.name}_FieldUpdate*)context;",
                    "    if (state == 0 || update == 0) {",
                    "        return;",
                    "    }",
                    f"    state->{field.name} = update->value;",
                    "}",
                    "",
                    f"int {prefix}_{snake}_set_{field.name}(KekStateStore* store, size_t slot_id, {c_type(field.type_name)} value) {{",
                    f"    {state.name}_{field.name}_FieldUpdate update = {{value}};",
                    f"    return kek_state_store_update_fields(store, slot_id, {prefix}_{snake}_set_{field.name}_update, &update, {macro}_FIELD_{field.name.upper()});",
                    "}",
                    "",
                    f"int {prefix}_set_{snake}_slot_{field.name}({runtime_name}* runtime, size_t slot_id, {c_type(field.type_name)} value) {{",
                    f"    return {prefix}_{snake}_set_{field.name}({prefix}_get_store(runtime), slot_id, value);",
                    "}",
                    "",
                ]
            )

    for instance in instances:
        snake = c_identifier_from_type(instance.state_name)
        instance_state = state_by_name.get(instance.state_name)
        lines.extend(
            [
                f"{instance.state_name}* {prefix}_{instance.name}(KekStateStore* store, const {slots_name}* slots) {{",
                "    if (slots == 0) {",
                "        return 0;",
                "    }",
                f"    return {prefix}_{snake}_slot(store, slots->{instance.name});",
                "}",
                "",
                f"const {instance.state_name}* {prefix}_{instance.name}_const(const KekStateStore* store, const {slots_name}* slots) {{",
                "    if (slots == 0) {",
                "        return 0;",
                "    }",
                f"    return {prefix}_{snake}_slot_const(store, slots->{instance.name});",
                "}",
                "",
                f"{instance.state_name}* {prefix}_{instance.name}_current({runtime_name}* runtime) {{",
                f"    return {prefix}_{instance.name}({prefix}_get_store(runtime), {prefix}_get_slots(runtime));",
                "}",
                "",
                f"const {instance.state_name}* {prefix}_{instance.name}_current_const(const {runtime_name}* runtime) {{",
                f"    return {prefix}_{instance.name}_const({prefix}_get_store_const(runtime), {prefix}_get_slots_const(runtime));",
                "}",
                "",
                f"size_t {prefix}_{instance.name}_slot_id(const {runtime_name}* runtime) {{",
                f"    const {slots_name}* slots = {prefix}_get_slots_const(runtime);",
                f"    return slots ? slots->{instance.name} : KEK_STATE_INVALID_ID;",
                "}",
                "",
                f"int {prefix}_update_{instance.name}({runtime_name}* runtime, KekStateStorageUpdateFn update, void* context, uint64_t changed_fields) {{",
                f"    return {prefix}_update_{snake}_slot(runtime, {prefix}_{instance.name}_slot_id(runtime), update, context, changed_fields);",
                "}",
                "",
                f"{item_name} {prefix}_{instance.name}_update_item({runtime_name}* runtime, KekStateStorageUpdateFn update, void* context, uint64_t changed_fields) {{",
                f"    return {prefix}_{snake}_slot_update_item({prefix}_{instance.name}_slot_id(runtime), update, context, changed_fields);",
                "}",
                "",
            ]
        )
        if instance_state is not None:
            for field in instance_state.fields:
                if field.array_length is not None or field.type_name == "String":
                    continue
                lines.extend(
                    [
                        f"int {prefix}_set_{instance.name}_{field.name}({runtime_name}* runtime, {c_type(field.type_name)} value) {{",
                        f"    return {prefix}_set_{snake}_slot_{field.name}(runtime, {prefix}_{instance.name}_slot_id(runtime), value);",
                        "}",
                        "",
                    ]
                )
    declared_state_names = sorted({instance.state_name for instance in instances if c_identifier_from_type(instance.state_name) != instance.name})
    for state_name in declared_state_names:
        state_instances = [instance for instance in instances if instance.state_name == state_name]
        if not state_instances:
            continue
        snake = c_identifier_from_type(state_name)
        lines.extend(
            [
                f"int {prefix}_is_declared_{snake}_slot(const {runtime_name}* runtime, size_t slot_id) {{",
                f"    const {slots_name}* slots = {prefix}_get_slots_const(runtime);",
                "    if (slots == 0) {",
                "        return 0;",
                "    }",
            ]
        )
        conditions = " || ".join(f"slot_id == slots->{instance.name}" for instance in state_instances)
        lines.append(f"    return {conditions};")
        lines.extend(["}", ""])
    for state in states:
        string_fields = [field for field in state.fields if field.type_name == "String" and field.array_length is None]
        if len(string_fields) != 1:
            continue
        snake = c_identifier_from_type(state.name)
        macro = state_type_macro(state.name)
        field_name = string_fields[0].name
        lines.extend(
            [
                f"typedef struct {state.name}_{field_name}_TextUpdate {{",
                "    const char* data;",
                "    size_t len;",
                f"}} {state.name}_{field_name}_TextUpdate;",
                "",
                f"static void {prefix}_{snake}_update_{field_name}(void* draft, void* context) {{",
                f"    {state.name}_{field_name}_TextUpdate* update = ({state.name}_{field_name}_TextUpdate*)context;",
                f"    {state.name}* state = ({state.name}*)draft;",
                f"    state->{field_name} = (KekString){{update->data, update->len}};",
                "}",
                "",
                f"int {prefix}_{snake}_set_{field_name}(KekStateStore* store, size_t slot_id, const char* data, size_t len) {{",
                f"    const KekStateDescriptor* descriptor = kek_state_store_descriptor(store, slot_id);",
                f"    if (descriptor == 0 || descriptor->type_id != {macro}) {{",
                "        return 0;",
                "    }",
                f"    {state.name}_{field_name}_TextUpdate update = {{data, len}};",
                f"    return kek_state_store_update_fields(store, slot_id, {prefix}_{snake}_update_{field_name}, &update, {macro}_FIELD_{field_name.upper()});",
                "}",
                "",
            ]
        )
    return "\n".join(lines).rstrip()


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
        if hook.instance_name is None:
            lines.append("        .state_slot_id = KEK_HOOK_ANY_SLOT,")
        else:
            lines.append("        .state_slot_id = KEK_HOOK_UNRESOLVED_SLOT,")
        lines.append(f"        .trigger_fields = {render_hook_trigger_fields(hook)},")
        lines.append(f"        .reads = {reads_name},")
        lines.append(f"        .read_count = {len(hook.reads)},")
        lines.append(f"        .writes = {writes_name},")
        lines.append(f"        .write_count = {len(hook.writes)},")
        lines.append(f"        .run = {hook.name},")
        lines.append("    },")
    lines.append("};")
    lines.append("const KekHookDescriptor* KekGeneratedHookDescriptors = KekGeneratedHookDescriptorData;")
    return "\n".join(lines)


def render_hook_trigger_fields(hook: Hook) -> str:
    if not hook.trigger_fields:
        return "KEK_EVENT_CHANGED_FIELDS_NONE"
    return " | ".join(
        f"{state_type_macro(hook.state_name)}_FIELD_{field_name.upper()}"
        for field_name in hook.trigger_fields
    )


def render_graph_body(states: list[State], hooks: list[Hook], instances: list[Instance]) -> str:
    lines: list[str] = []
    for state in states:
        lines.append(f"    S_{state.name}[\"{state.name}\"]")
    for hook in hooks:
        lines.append(f"    H_{hook.name}{{\"{hook.name}\"}}")
    for instance in instances:
        lines.append(f"    I_{instance.name}([\"{instance.name}: {instance.state_name}\"])")
        lines.append(f"    S_{instance.state_name} -. instance .-> I_{instance.name}")
    for hook in hooks:
        event_name = hook.event_type.removeprefix("KEK_EVENT_STATE_").lower()
        if hook.instance_name is None:
            lines.append(f"    S_{hook.state_name} -->|{event_name}| H_{hook.name}")
        else:
            lines.append(f"    I_{hook.instance_name} -->|{event_name}| H_{hook.name}")
        for state_name in hook.reads:
            lines.append(f"    S_{state_name} -. reads .-> H_{hook.name}")
        for state_name in hook.writes:
            lines.append(f"    H_{hook.name} -->|writes| S_{state_name}")
    return "\n".join(lines)
