from __future__ import annotations

import json
import re
from pathlib import Path

ANALYSIS_COMPILE_DB_DIR_NAME = "analysis_compile_db"
CMAKE_CACHE_KEY_ANALYSIS_COMPILE_DB_DIR = "TT_ANALYSIS_COMPILE_DB_DIR"

_MODMAP_ARG_PATTERN = re.compile(r"(?P<prefix>\s)(?P<arg>@[^\"\s]+\.obj\.modmap)")


def resolve_compile_db_dir(build_dir: Path) -> Path:
    return build_dir / ANALYSIS_COMPILE_DB_DIR_NAME


def resolve_compile_db_path(build_dir: Path) -> Path:
    return resolve_compile_db_dir(build_dir) / "compile_commands.json"


def resolve_compile_db_cache_value(build_dir: Path) -> str:
    return str(resolve_compile_db_dir(build_dir).resolve()).replace("\\", "/")


def load_compile_units(compile_commands_path: Path) -> list[Path]:
    try:
        raw_payload = json.loads(
            compile_commands_path.read_text(encoding="utf-8", errors="replace")
        )
    except (OSError, json.JSONDecodeError):
        return []

    if not isinstance(raw_payload, list):
        return []

    files_by_key: dict[str, Path] = {}
    for item in raw_payload:
        if not isinstance(item, dict):
            continue
        file_raw = item.get("file")
        if not isinstance(file_raw, str) or not file_raw.strip():
            continue
        file_path = Path(file_raw)
        if not file_path.is_absolute():
            directory_raw = item.get("directory")
            if isinstance(directory_raw, str) and directory_raw.strip():
                file_path = Path(directory_raw) / file_path
        files_by_key[_path_key(file_path)] = file_path
    return sorted(files_by_key.values(), key=_path_key)


def _resolve_modmap_path(arg_text: str, directory: str | None) -> Path | None:
    if not arg_text.startswith("@"):
        return None
    modmap_text = arg_text[1:]
    if not modmap_text:
        return None
    modmap_path = Path(modmap_text)
    if modmap_path.is_absolute() or directory is None:
        return modmap_path
    return Path(directory) / modmap_path


def _should_keep_modmap_arg(arg_text: str, directory: str | None) -> bool:
    modmap_path = _resolve_modmap_path(arg_text, directory)
    if modmap_path is None:
        return False
    return modmap_path.exists()


def sanitize_command_text(command_text: str, directory: str | None = None) -> str:
    def _replace(match: re.Match[str]) -> str:
        arg_text = match.group("arg")
        if _should_keep_modmap_arg(arg_text, directory):
            return match.group(0)
        return ""

    return _MODMAP_ARG_PATTERN.sub(_replace, command_text)


def sanitize_compile_commands(entries: list[dict]) -> list[dict]:
    sanitized_entries: list[dict] = []
    for item in entries:
        next_item = dict(item)
        directory = next_item.get("directory")
        directory_text = str(directory) if isinstance(directory, str) else None
        command_text = next_item.get("command")
        if isinstance(command_text, str):
            next_item["command"] = sanitize_command_text(command_text, directory_text)

        arguments = next_item.get("arguments")
        if isinstance(arguments, list):
            next_item["arguments"] = [
                arg
                for arg in arguments
                if not (
                    isinstance(arg, str)
                    and arg.startswith("@")
                    and arg.endswith(".obj.modmap")
                    and not _should_keep_modmap_arg(arg, directory_text)
                )
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

    payload = json.loads(source_path.read_text(encoding="utf-8"))
    if not isinstance(payload, list):
        raise ValueError(f"invalid compile_commands payload: {source_path}")
    sanitized = sanitize_compile_commands(payload)
    target_path.write_text(
        json.dumps(sanitized, indent=2, ensure_ascii=False),
        encoding="utf-8",
    )
    return target_dir


def _path_key(path: Path) -> str:
    normalized = str(path).replace("\\", "/")
    while "//" in normalized:
        normalized = normalized.replace("//", "/")
    return normalized.lower()
