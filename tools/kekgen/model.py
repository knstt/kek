from dataclasses import dataclass, field


@dataclass
class Enum:
    name: str
    values: list[str] = field(default_factory=list)


@dataclass
class Field:
    name: str
    type_name: str
    default: object
    minimum: object | None = None
    maximum: object | None = None
    array_length: int | None = None


@dataclass
class State:
    name: str
    fields: list[Field] = field(default_factory=list)
    constructors: list["Constructor"] = field(default_factory=list)
    pool_capacity: int = 0


@dataclass
class Constructor:
    name: str
    values: dict[str, object] = field(default_factory=dict)


@dataclass
class Instance:
    name: str
    state_name: str
    constructor_name: str | None = None
    values: dict[str, object] = field(default_factory=dict)
    config: dict[str, object] = field(default_factory=dict)


@dataclass
class HookAccess:
    mode: str
    state_name: str
    instance_name: str | None = None
    scope: str = "any"
    fields: list[str] = field(default_factory=list)


@dataclass
class Hook:
    name: str
    event_type: str = ""
    state_name: str = ""
    instance_name: str | None = None
    trigger_fields: list[str] = field(default_factory=list)
    reads: list[str] = field(default_factory=list)
    writes: list[str] = field(default_factory=list)
    accesses: list[HookAccess] = field(default_factory=list)
    access_declared: bool = False
    scheduling_opaque: bool = False
    field_merge_safe: bool = False
    needs_event_state: bool = False
