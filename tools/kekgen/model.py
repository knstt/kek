from dataclasses import dataclass, field


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
