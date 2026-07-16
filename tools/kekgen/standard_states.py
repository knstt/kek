STANDARD_STATES = [
    {
        "id": "std.input",
        "name": "StandardInput",
        "description": "Text received from the runtime standard input bridge.",
        "state": {
            "name": "StandardInput",
            "fields": [
                {"name": "input", "type": "String", "default": "", "max": 1000},
            ],
            "constructors": [],
        },
        "instance": {"name": "standard_input", "state": "StandardInput"},
    },
    {
        "id": "std.output",
        "name": "StandardOutput",
        "description": "Text written through the runtime standard output bridge.",
        "state": {
            "name": "StandardOutput",
            "fields": [
                {"name": "output", "type": "String", "default": "", "max": 1000},
            ],
            "constructors": [],
        },
        "instance": {"name": "standard_output", "state": "StandardOutput"},
    },
    {
        "id": "std.timer",
        "name": "Timer",
        "description": "Timer tick state for a future runtime timer bridge.",
        "state": {
            "name": "Timer",
            "fields": [
                {"name": "tick", "type": "u64", "default": 0},
                {"name": "interval_ms", "type": "u32", "default": 1000, "min": 1},
                {"name": "enabled", "type": "bool", "default": True},
            ],
            "constructors": [],
        },
        "instance": {"name": "timer", "state": "Timer"},
    },
]

def standard_states_payload() -> list[dict]:
    return STANDARD_STATES


def standard_states_by_type() -> dict[str, dict]:
    return {item["state"]["name"]: item for item in STANDARD_STATES}
