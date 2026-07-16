#!/usr/bin/env python3
import argparse
import json
import mimetypes
import os
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

import generate_states


ROOT_DIR = Path(__file__).resolve().parent.parent
EDITOR_DIR = ROOT_DIR / "editor"


def state_to_dict(state: generate_states.State) -> dict:
    return {
        "name": state.name,
        "fields": [
            {
                key: value
                for key, value in {
                    "name": item.name,
                    "type": item.type_name,
                    "default": item.default,
                    "min": item.minimum,
                    "max": item.maximum,
                    "array": item.array_length,
                }.items()
                if value is not None
            }
            for item in state.fields
        ],
        "constructors": [{"name": item.name, "values": item.values} for item in state.constructors],
    }


def enum_to_dict(enum: generate_states.Enum) -> dict:
    return {
        "name": enum.name,
        "values": enum.values,
    }


def hook_to_dict(hook: generate_states.Hook) -> dict:
    return {
        "name": hook.name,
        "on": {
            "state": hook.state_name,
            "event": "changed" if hook.event_type == "KEK_EVENT_STATE_CHANGED" else hook.event_type,
        },
        "reads": hook.reads,
        "writes": hook.writes,
    }


def instance_to_dict(instance: generate_states.Instance) -> dict:
    result = {
        "name": instance.name,
        "state": instance.state_name,
    }
    if instance.constructor_name is not None:
        result["constructor"] = instance.constructor_name
    return result


def model_from_source(source: str) -> dict:
    enums, states, hooks, instances = generate_states.parse_source(source)
    validate_source_model(enums, states, hooks, instances)
    return {
        "version": 1,
        "enums": [enum_to_dict(enum) for enum in enums],
        "states": [state_to_dict(state) for state in states],
        "instances": [instance_to_dict(instance) for instance in instances],
        "hooks": [hook_to_dict(hook) for hook in hooks],
    }


def validate_source_model(
    enums: list[generate_states.Enum],
    states: list[generate_states.State],
    hooks: list[generate_states.Hook],
    instances: list[generate_states.Instance],
) -> None:
    generate_states.emit_source(states, hooks, instances, "validation", enums=enums)


def render_source(model: dict) -> str:
    return json.dumps(model, indent=2) + "\n"


class EditorServer(ThreadingHTTPServer):
    def __init__(self, address: tuple[str, int], project_dir: Path):
        super().__init__(address, EditorHandler)
        self.project_dir = project_dir.resolve()


class EditorHandler(BaseHTTPRequestHandler):
    server: EditorServer

    def log_message(self, format: str, *args: object) -> None:
        print(f"{self.address_string()} - {format % args}")

    def send_json(self, status: int, payload: dict) -> None:
        body = json.dumps(payload, indent=2).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def send_text(self, status: int, text: str, content_type: str = "text/plain; charset=utf-8") -> None:
        body = text.encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def read_json_body(self) -> dict:
        length = int(self.headers.get("Content-Length", "0"))
        if length == 0:
            return {}
        return json.loads(self.rfile.read(length).decode("utf-8"))

    def project_path(self, value: str | None) -> Path:
        if not value:
            raise ValueError("missing file parameter")
        candidate = (self.server.project_dir / value).resolve()
        if self.server.project_dir != candidate and self.server.project_dir not in candidate.parents:
            raise ValueError("path escapes project directory")
        if candidate.suffix != ".json":
            raise ValueError("schema file must use .json extension")
        return candidate

    def do_GET(self) -> None:
        parsed = urlparse(self.path)
        if parsed.path == "/api/project":
            files = sorted(
                str(path.relative_to(self.server.project_dir))
                for path in self.server.project_dir.rglob("*.json")
                if path.is_file()
            )
            self.send_json(200, {"project": str(self.server.project_dir), "files": files})
            return

        if parsed.path == "/api/standard-states":
            self.send_json(200, {"standardStates": generate_states.standard_states_payload()})
            return

        if parsed.path == "/api/schema":
            params = parse_qs(parsed.query)
            try:
                schema_path = self.project_path(params.get("file", [None])[0])
                source = schema_path.read_text(encoding="utf-8")
                self.send_json(200, {"file": str(schema_path.relative_to(self.server.project_dir)), "source": source, "model": model_from_source(source)})
            except (OSError, SyntaxError, ValueError) as error:
                self.send_json(400, {"error": str(error)})
            return

        self.serve_static(parsed.path)

    def do_POST(self) -> None:
        parsed = urlparse(self.path)
        params = parse_qs(parsed.query)

        if parsed.path == "/api/schema":
            try:
                schema_path = self.project_path(params.get("file", [None])[0])
                payload = self.read_json_body()
                source = payload.get("source")
                if source is None:
                    source = render_source(payload.get("model", {}))
                model_from_source(source)
                schema_path.parent.mkdir(parents=True, exist_ok=True)
                schema_path.write_text(source, encoding="utf-8")
                self.send_json(200, {"ok": True, "file": str(schema_path.relative_to(self.server.project_dir)), "source": source})
            except (OSError, SyntaxError, ValueError, json.JSONDecodeError) as error:
                self.send_json(400, {"error": str(error)})
            return

        if parsed.path == "/api/render":
            try:
                payload = self.read_json_body()
                source = payload.get("source")
                if source is None:
                    source = render_source(payload.get("model", {}))
                model = model_from_source(source)
                self.send_json(200, {"source": source, "model": model})
            except (SyntaxError, ValueError, json.JSONDecodeError) as error:
                self.send_json(400, {"error": str(error)})
            return

        if parsed.path == "/api/generate":
            try:
                schema_path = self.project_path(params.get("file", [None])[0])
                source = schema_path.read_text(encoding="utf-8")
                out_dir = ROOT_DIR / "generated"
                base_name = schema_path.stem
                header_path, source_path, graph_path = generate_states.write_generated_files(source, out_dir, base_name)
                self.send_json(200, {"ok": True, "files": [str(path.relative_to(ROOT_DIR)) for path in (header_path, source_path, graph_path)]})
            except (OSError, SyntaxError, ValueError) as error:
                self.send_json(400, {"error": str(error)})
            return

        self.send_json(404, {"error": "not found"})

    def serve_static(self, request_path: str) -> None:
        relative = "index.html" if request_path in ("", "/") else request_path.lstrip("/")
        static_path = (EDITOR_DIR / relative).resolve()
        if EDITOR_DIR != static_path and EDITOR_DIR not in static_path.parents:
            self.send_text(403, "forbidden")
            return
        if not static_path.is_file():
            self.send_text(404, "not found")
            return
        content_type = mimetypes.guess_type(static_path.name)[0] or "application/octet-stream"
        body = static_path.read_bytes()
        self.send_response(200)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def main() -> int:
    parser = argparse.ArgumentParser(description="Run the local Kek browser editor.")
    parser.add_argument("project", nargs="?", default="examples/game", help="project folder containing JSON schema files")
    parser.add_argument("--host", default="127.0.0.1", help="bind host")
    parser.add_argument("--port", type=int, default=8080, help="bind port")
    args = parser.parse_args()

    project_dir = Path(args.project).resolve()
    if not project_dir.is_dir():
        print(f"kek_editor.py: project folder does not exist: {project_dir}")
        return 1

    server = EditorServer((args.host, args.port), project_dir)
    print(f"Kek editor serving {project_dir} at http://{args.host}:{args.port}/")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nKek editor stopped")
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
