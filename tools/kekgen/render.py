import json
from importlib import resources
from string import Template

from .model import Field, Hook, Instance, State
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


def emit_header(states: list[State], hooks: list[Hook], instances: list[Instance] | str | None = None, name: str | None = None) -> str:
    if name is None:
        name = str(instances)
        instances = []
    instances = instances if isinstance(instances, list) else []
    return render_template(
        "generated.h.tpl",
        guard=generated_guard(name),
        state_type_enum=render_state_type_enum(states),
        state_declarations=render_state_declarations(states),
        aggregate_declarations=render_aggregate_declarations(states, name),
        instance_declarations=render_instance_declarations(states, instances, name),
        hook_count=len(hooks),
        hook_declarations=render_hook_declarations(hooks),
    )


def emit_source(states: list[State], hooks: list[Hook], instances: list[Instance] | str | None = None, name: str | None = None) -> str:
    if name is None:
        name = str(instances)
        instances = []
    instances = instances if isinstance(instances, list) else []
    aggregate_name = aggregate_state_name(name)
    return render_template(
        "generated.c.tpl",
        header_name=f"{name}.h",
        state_definitions=render_state_definitions(states),
        aggregate_name=aggregate_name,
        aggregate_field_defaults=render_aggregate_field_defaults(states),
        aggregate_checks=render_aggregate_checks(states),
        descriptor_entries=render_descriptor_entries(states),
        instance_definitions=render_instance_definitions(states, instances, name),
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


def generated_prefix(name: str) -> str:
    return c_identifier_from_type(name)


def instance_slots_name(name: str) -> str:
    return f"{name[:1].upper()}{name[1:]}StateSlots"


def runtime_binding_name(name: str) -> str:
    return f"{name[:1].upper()}{name[1:]}RuntimeBinding"


def render_instance_declarations(states: list[State], instances: list[Instance], name: str) -> str:
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
    lines.append(f"int {prefix}_state_slots_add_declared(KekStateStore* store, {slots_name}* slots);")
    lines.append(f"int {prefix}_state_slots_remove_declared(KekStateStore* store, {slots_name}* slots);")
    lines.append("")
    binding_name = runtime_binding_name(name)
    lines.append(f"typedef struct {binding_name} {{")
    lines.append("    KekRuntime* runtime;")
    lines.append("    KekStateStore state_store;")
    lines.append("    KekHookRegistry hook_registry;")
    lines.append(f"    {slots_name} slots;")
    lines.append(f"}} {binding_name};")
    lines.append("")
    lines.append(f"int {prefix}_runtime_binding_init({binding_name}* binding, KekRuntime* runtime, void* app_context);")
    lines.append(f"void {prefix}_runtime_binding_destroy({binding_name}* binding);")
    lines.append("")
    for state in states:
        snake = c_identifier_from_type(state.name)
        macro = state_type_macro(state.name)
        lines.append(f"size_t {prefix}_{snake}_create(KekStateStore* store);")
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
        string_fields = [field for field in state.fields if field.type_name == "String"]
        if len(string_fields) == 1:
            snake = c_identifier_from_type(state.name)
            field_name = string_fields[0].name
            lines.append(
                f"int {prefix}_{snake}_set_{field_name}(KekStateStore* store, size_t slot_id, const char* data, size_t len);"
            )
    return "\n".join(lines).rstrip()


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


def render_instance_definitions(states: list[State], instances: list[Instance], name: str) -> str:
    prefix = generated_prefix(name)
    slots_name = instance_slots_name(name)
    binding_name = runtime_binding_name(name)
    state_by_name = {state.name: state for state in states}
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
            f"int {prefix}_state_slots_add_declared(KekStateStore* store, {slots_name}* slots) {{",
            "    if (store == 0 || slots == 0) {",
            "        return 0;",
            "    }",
            f"    {prefix}_state_slots_init_invalid(slots);",
        ]
    )
    for instance in instances:
        state = state_by_name[instance.state_name]
        initial_expr = "0"
        if instance.constructor_name is not None:
            lines.append(f"    {state.name} {instance.name}_initial = {state.name}_{instance.constructor_name}();")
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
            f"int {prefix}_runtime_binding_init({binding_name}* binding, KekRuntime* runtime, void* app_context) {{",
            "    if (binding == 0 || runtime == 0) {",
            "        return 0;",
            "    }",
            "    memset(binding, 0, sizeof(*binding));",
            "    binding->runtime = runtime;",
            "    kek_state_store_init(&binding->state_store, runtime);",
            f"    if (!{prefix}_state_slots_add_declared(&binding->state_store, &binding->slots)) {{",
            "        kek_state_store_destroy(&binding->state_store);",
            "        memset(binding, 0, sizeof(*binding));",
            "        return 0;",
            "    }",
            "    kek_hook_registry_init(&binding->hook_registry, runtime, &binding->state_store, app_context);",
            "    if (!kek_hook_registry_add_many(&binding->hook_registry, KekGeneratedHookDescriptors, KEK_GENERATED_HOOK_COUNT)) {",
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
        string_fields = [field for field in state.fields if field.type_name == "String"]
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
        lines.append(f"    S_{hook.state_name} -->|changed| H_{hook.name}")
        for state_name in hook.reads:
            lines.append(f"    S_{state_name} -. reads .-> H_{hook.name}")
        for state_name in hook.writes:
            lines.append(f"    H_{hook.name} -->|writes| S_{state_name}")
    return "\n".join(lines)
