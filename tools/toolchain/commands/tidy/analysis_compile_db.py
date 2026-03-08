from __future__ import annotations

import json
import re
from pathlib import Path

ANALYSIS_COMPILE_DB_DIR_NAME = "analysis_compile_db"
CMAKE_CACHE_KEY_ANALYSIS_COMPILE_DB_DIR = "TT_ANALYSIS_COMPILE_DB_DIR"

_MODMAP_ARG_PATTERN = re.compile(r"\s@[^\"\s]+\.obj\.modmap")


def resolve_compile_db_dir(build_dir: Path) -> Path:
    return build_dir / ANALYSIS_COMPILE_DB_DIR_NAME


def resolve_compile_db_path(build_dir: Path) -> Path:
    return resolve_compile_db_dir(build_dir) / "compile_commands.json"


def resolve_compile_db_cache_value(build_dir: Path) -> str:
    return str(resolve_compile_db_dir(build_dir).resolve()).replace("\\", "/")


def sanitize_command_text(command_text: str) -> str:
    return _MODMAP_ARG_PATTERN.sub("", command_text)


def sanitize_compile_commands(entries: list[dict]) -> list[dict]:
    sanitized_entries: list[dict] = []
    for item in entries:
        next_item = dict(item)
        command_text = next_item.get("command")
        if isinstance(command_text, str):
            next_item["command"] = sanitize_command_text(command_text)

        arguments = next_item.get("arguments")
        if isinstance(arguments, list):
            next_item["arguments"] = [
                arg
                for arg in arguments
                if not (isinstance(arg, str) and arg.startswith("@") and arg.endswith(".obj.modmap"))
            ]
        sanitized_entries.append(next_item)
    return sanitized_entries


def ensure_analysis_compile_db(build_dir: Path) -> Path:
    source_path = build_dir / "compile_commands.json"
    if not source_path.exists():
        raise FileNotFoundError(f"compile_commands.json not found: {source_path}")

    target_dir = resolve_compile_db_dir(build_dir)
    target_path = target_dir / "compile_commands.json"
    target_dir.mkdir(parents=True, exist_ok=True)

    if target_path.exists():
        try:
            if target_path.stat().st_mtime_ns >= source_path.stat().st_mtime_ns:
                return target_dir
        except OSError:
            pass

    payload = json.loads(source_path.read_text(encoding="utf-8"))
    if not isinstance(payload, list):
        raise ValueError(f"invalid compile_commands payload: {source_path}")
    sanitized = sanitize_compile_commands(payload)
    target_path.write_text(
        json.dumps(sanitized, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    return target_dir
