from dataclasses import dataclass
from pathlib import Path

from .model import Hook, Instance, State
from .parser import parse_source
from .render import emit_graph, emit_header, emit_source


@dataclass
class GeneratedFiles:
    header: str
    source: str
    graph: str


def render_all(states: list[State], hooks: list[Hook], name: str) -> GeneratedFiles:
    return GeneratedFiles(
        emit_header(states, hooks, [], name),
        emit_source(states, hooks, [], name),
        emit_graph(states, hooks, [], name),
    )


def render_all_model(states: list[State], hooks: list[Hook], instances: list[Instance], name: str) -> GeneratedFiles:
    return GeneratedFiles(
        emit_header(states, hooks, instances, name),
        emit_source(states, hooks, instances, name),
        emit_graph(states, hooks, instances, name),
    )


def write_generated_files(source: str, out_dir: str | Path, name: str) -> tuple[Path, Path, Path]:
    states, hooks, instances = parse_source(source)
    generated = render_all_model(states, hooks, instances, name)

    output_dir = Path(out_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    header_path = output_dir / f"{name}.h"
    source_path = output_dir / f"{name}.c"
    graph_path = output_dir / f"{name}.graph.md"

    header_path.write_text(generated.header, encoding="utf-8")
    source_path.write_text(generated.source, encoding="utf-8")
    graph_path.write_text(generated.graph, encoding="utf-8")
    return header_path, source_path, graph_path
