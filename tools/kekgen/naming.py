import re


def require_identifier(value: object, label: str) -> str:
    if not isinstance(value, str) or not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", value):
        raise ValueError(f"{label} must be an identifier")
    return value


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
    return f"KEK_STATE_TYPE_{c_identifier_from_type(name).upper()}"
