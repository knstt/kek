import json

from .model import Constructor, Enum, Field, Hook, Instance, State
from .naming import require_identifier


def parse_source(source: str) -> tuple[list[Enum], list[State], list[Hook], list[Instance]]:
    try:
        document = json.loads(source)
    except json.JSONDecodeError as error:
        raise SyntaxError(f"invalid JSON: {error.msg} at line {error.lineno}") from error
    return parse_document(document)


def parse_document(document: object) -> tuple[list[Enum], list[State], list[Hook], list[Instance]]:
    if not isinstance(document, dict):
        raise ValueError("schema root must be an object")

    state_items = document.get("states")
    if not isinstance(state_items, list) or not state_items:
        raise ValueError("schema must contain one or more states")

    enums = parse_enums(document.get("enums", []))
    states = parse_states(state_items)
    hooks = parse_hooks(document.get("hooks", []))
    instances = parse_instances(document.get("instances", []))
    validate_field_values(enums, states)
    validate_hooks(states, hooks)
    validate_instances(states, instances)
    return enums, states, hooks, instances


def parse_enums(enum_items: object) -> list[Enum]:
    if not isinstance(enum_items, list):
        raise ValueError("enums must be an array")

    enums: list[Enum] = []
    enum_names: set[str] = set()
    for enum_index, enum_item in enumerate(enum_items):
        if not isinstance(enum_item, dict):
            raise ValueError(f"enums[{enum_index}] must be an object")
        enum_name = require_identifier(enum_item.get("name"), f"enums[{enum_index}].name")
        if enum_name in enum_names:
            raise ValueError(f"duplicate enum {enum_name}")
        values_item = enum_item.get("values")
        if not isinstance(values_item, list) or not values_item:
            raise ValueError(f"{enum_name}: enum must contain one or more values")
        values: list[str] = []
        value_names: set[str] = set()
        for value_index, value_item in enumerate(values_item):
            value_name = require_identifier(value_item, f"{enum_name}.values[{value_index}]")
            if value_name in value_names:
                raise ValueError(f"{enum_name}: duplicate enum value {value_name}")
            value_names.add(value_name)
            values.append(value_name)
        enum_names.add(enum_name)
        enums.append(Enum(enum_name, values))
    return enums


def parse_states(state_items: list[object]) -> list[State]:
    states: list[State] = []
    state_names: set[str] = set()
    for state_index, state_item in enumerate(state_items):
        if not isinstance(state_item, dict):
            raise ValueError(f"states[{state_index}] must be an object")
        state_name = require_identifier(state_item.get("name"), f"states[{state_index}].name")
        if state_name in state_names:
            raise ValueError(f"duplicate state {state_name}")
        state_names.add(state_name)

        fields, field_names = parse_fields(state_name, state_item.get("fields"))
        constructors = parse_constructors(state_name, state_item.get("constructors", []), field_names)
        states.append(State(state_name, fields, constructors))
    return states


def parse_fields(state_name: str, field_items: object) -> tuple[list[Field], set[str]]:
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
        array_length = parse_array_length(field_item.get("array"), f"{state_name}.{field_name}.array")
        field_names.add(field_name)
        fields.append(
            Field(
                field_name,
                require_identifier(field_item.get("type"), f"{state_name}.{field_name}.type"),
                field_item.get("default"),
                field_item.get("min"),
                field_item.get("max"),
                array_length,
            )
        )
    return fields, field_names


def parse_array_length(value: object, label: str) -> int | None:
    if value is None:
        return None
    if not isinstance(value, int) or isinstance(value, bool) or value <= 0:
        raise ValueError(f"{label} must be a positive integer")
    return value


def parse_constructors(
    state_name: str,
    constructor_items: object,
    field_names: set[str],
) -> list[Constructor]:
    if not isinstance(constructor_items, list):
        raise ValueError(f"{state_name}.constructors must be an array")

    constructors: list[Constructor] = []
    constructor_names: set[str] = set()
    for constructor_index, constructor_item in enumerate(constructor_items):
        if not isinstance(constructor_item, dict):
            raise ValueError(f"{state_name}.constructors[{constructor_index}] must be an object")
        constructor_name = require_identifier(
            constructor_item.get("name"),
            f"{state_name}.constructors[{constructor_index}].name",
        )
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
    return constructors


def parse_hooks(hook_items: object) -> list[Hook]:
    if not isinstance(hook_items, list):
        raise ValueError("hooks must be an array")

    hooks: list[Hook] = []
    hook_names: set[str] = set()
    for hook_index, hook_item in enumerate(hook_items):
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
        hook_names.add(hook_name)
        hooks.append(
            Hook(
                hook_name,
                "KEK_EVENT_STATE_CHANGED",
                require_identifier(on_item.get("state"), f"{hook_name}.on.state"),
                parse_json_name_list(hook_item.get("reads", []), f"{hook_name}.reads"),
                parse_json_name_list(hook_item.get("writes", []), f"{hook_name}.writes"),
            )
        )
    return hooks


def parse_instances(instance_items: object) -> list[Instance]:
    if not isinstance(instance_items, list):
        raise ValueError("instances must be an array")

    instances: list[Instance] = []
    instance_names: set[str] = set()
    for instance_index, instance_item in enumerate(instance_items):
        if not isinstance(instance_item, dict):
            raise ValueError(f"instances[{instance_index}] must be an object")
        instance_name = require_identifier(
            instance_item.get("name"),
            f"instances[{instance_index}].name",
        )
        if instance_name in instance_names:
            raise ValueError(f"duplicate instance {instance_name}")
        state_name = require_identifier(
            instance_item.get("state"),
            f"instances[{instance_index}].state",
        )
        constructor_value = instance_item.get("constructor")
        constructor_name = None
        if constructor_value is not None:
            constructor_name = require_identifier(
                constructor_value,
                f"instances[{instance_index}].constructor",
            )
        instance_names.add(instance_name)
        instances.append(Instance(instance_name, state_name, constructor_name))
    return instances


def parse_json_name_list(value: object, label: str) -> list[str]:
    if not isinstance(value, list):
        raise ValueError(f"{label} must be an array")
    return [require_identifier(item, f"{label}[]") for item in value]


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


def validate_field_values(enums: list[Enum], states: list[State]) -> None:
    enum_values = {enum.name: set(enum.values) for enum in enums}
    for state in states:
        fields = field_map(state)
        for field_item in state.fields:
            validate_field_default(enum_values, state.name, field_item, field_item.default)
        for constructor in state.constructors:
            for field_name, value in constructor.values.items():
                validate_field_default(enum_values, f"{state.name}.{constructor.name}", fields[field_name], value)


def field_map(state: State) -> dict[str, Field]:
    return {field_item.name: field_item for field_item in state.fields}


def validate_field_default(enum_values: dict[str, set[str]], owner: str, field_item: Field, value: object) -> None:
    if field_item.array_length is None:
        validate_scalar_default(enum_values, owner, field_item, value)
        return
    if not isinstance(value, list):
        raise ValueError(f"{owner}.{field_item.name}: array default must be an array")
    if len(value) != field_item.array_length:
        raise ValueError(f"{owner}.{field_item.name}: array default must contain {field_item.array_length} values")
    scalar_field = Field(field_item.name, field_item.type_name, None, field_item.minimum, field_item.maximum)
    for index, item in enumerate(value):
        validate_scalar_default(enum_values, f"{owner}.{field_item.name}[{index}]", scalar_field, item)


def validate_scalar_default(enum_values: dict[str, set[str]], owner: str, field_item: Field, value: object) -> None:
    values = enum_values.get(field_item.type_name)
    if values is not None and value not in values:
        raise ValueError(f"{owner}.{field_item.name}: enum default must be one of {sorted(values)}")


def validate_instances(states: list[State], instances: list[Instance]) -> None:
    state_by_name = {state.name: state for state in states}
    for instance in instances:
        state_item = state_by_name.get(instance.state_name)
        if state_item is None:
            raise ValueError(f"{instance.name}: instance references unknown state {instance.state_name}")
        if instance.constructor_name is None:
            continue
        constructor_names = {constructor.name for constructor in state_item.constructors}
        if instance.constructor_name not in constructor_names:
            raise ValueError(
                f"{instance.name}: instance references unknown constructor "
                f"{instance.state_name}.{instance.constructor_name}"
            )
