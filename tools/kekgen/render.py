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
    lines.append(f"void {prefix}_state_slots_init_invalid({slots_name}* slots);")
    binding_name = runtime_binding_name(name)
    lines.append(f"typedef struct {binding_name} {binding_name};")
    lines.append(f"int {prefix}_state_slots_add_declared(KekStateStore* store, {slots_name}* slots, {binding_name}* binding);")
    lines.append(f"int {prefix}_state_slots_remove_declared(KekStateStore* store, {slots_name}* slots);")
    lines.append(f"int {prefix}_state_slots_reset_declared(KekStateStore* store, const {slots_name}* slots);")
    lines.append("")
    lines.append(f"struct {binding_name} {{")
    lines.append("    KekRuntime* runtime;")
    lines.append("    KekStateStore state_store;")
    lines.append("    KekHookRegistry hook_registry;")
    if hooks:
        lines.append(f"    KekHookDescriptor hook_descriptors[{len(hooks)}];")
    for instance in instances:
        state = next((item for item in states if item.name == instance.state_name), None)
        if state is None:
            continue
        for field in state.fields:
            if field.type_name == "String" and field.array_length is None and isinstance(field.maximum, int):
                lines.append(f"    char {instance.name}_{field.name}_buffer[{field.maximum + 1}];")
    lines.append(f"    {slots_name} slots;")
    lines.append("};")
    lines.append("")
    lines.append(f"int {prefix}_runtime_binding_init({binding_name}* binding, KekRuntime* runtime, void* app_context);")
    lines.append(f"void {prefix}_runtime_binding_destroy({binding_name}* binding);")
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
        lines.append(f"#define {prefix.upper()}_{snake.upper()}_STATE_TYPE {macro}")
        lines.append("")
    for instance in instances:
        lines.append(f"{instance.state_name}* {prefix}_{instance.name}(KekStateStore* store, const {slots_name}* slots);")
        lines.append(
            f"const {instance.state_name}* {prefix}_{instance.name}_const(const KekStateStore* store, const {slots_name}* slots);"
        )
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
    return "\n".join(f"void {hook.name}(KekHookContext* context);" for hook in hooks)


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
            f"int {prefix}_state_slots_add_declared(KekStateStore* store, {slots_name}* slots, {binding_name}* binding) {{",
            "    if (store == 0 || slots == 0 || binding == 0) {",
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
                lines.append(f"    {instance.name}_initial.{field.name} = (KekString){{binding->{instance.name}_{field.name}_buffer, 0}};")
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
            lines.append(f"        updates[count] = (KekStateStoreUpdateItem){{slots->{instance.name}, kek_generated_reset_slot, (void*)descriptors[count]}};")
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
            ]
        )

    for instance in instances:
        snake = c_identifier_from_type(instance.state_name)
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
            ]
        )
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
                f"    return kek_state_store_update(store, slot_id, {prefix}_{snake}_update_{field_name}, &update);",
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
        lines.append(f"        .reads = {reads_name},")
        lines.append(f"        .read_count = {len(hook.reads)},")
        lines.append(f"        .writes = {writes_name},")
        lines.append(f"        .write_count = {len(hook.writes)},")
        lines.append(f"        .run = {hook.name},")
        lines.append("    },")
    lines.append("};")
    lines.append("const KekHookDescriptor* KekGeneratedHookDescriptors = KekGeneratedHookDescriptorData;")
    return "\n".join(lines)


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
