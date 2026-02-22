import json
from pathlib import Path
from typing import Any, Optional


class TeeStream:
    def __init__(self, original_stream, mirror_stream):
        self.original_stream = original_stream
        self.mirror_stream = mirror_stream

    def write(self, data):
        if not data:
            return 0
        self.original_stream.write(data)
        self.mirror_stream.write(data)
        return len(data)

    def flush(self):
        self.original_stream.flush()
        self.mirror_stream.flush()

    def isatty(self):
        if hasattr(self.original_stream, "isatty"):
            return self.original_stream.isatty()
        return False

    @property
    def encoding(self):
        return getattr(self.original_stream, "encoding", "utf-8")


def resolve_path(
    path_value: Optional[str], base_dir: Path, default_relative: str
) -> Path:
    path_obj = Path(path_value) if path_value else base_dir / default_relative
    if not path_obj.is_absolute():
        path_obj = (base_dir / path_obj).resolve()
    return path_obj


def write_result_json(result_path: Path, result_payload: Any):
    result_path.parent.mkdir(parents=True, exist_ok=True)
    result_path.write_text(
        json.dumps(result_payload, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )
    print(f"Result JSON: {result_path}")


def resolve_python_output_log_path(
    result_payload: Any,
    fallback_log_dir: Path,
) -> Path:
    log_dir = fallback_log_dir
    if isinstance(result_payload, dict):
        configured_log_dir = result_payload.get("log_dir")
        if configured_log_dir:
            log_dir = Path(configured_log_dir)
    log_dir.mkdir(parents=True, exist_ok=True)
    return log_dir / "output.log"
