from __future__ import annotations

from typing import Any

from .path_utils import normalize_rel_path


def ensure_dict(value: Any, field_path: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ValueError(f"Expected table at '{field_path}'.")
    return value


def ensure_str(value: Any, field_path: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"Expected non-empty string at '{field_path}'.")
    return value


def ensure_list_of_str(value: Any, field_path: str) -> list[str]:
    if not isinstance(value, list):
        raise ValueError(f"Expected array at '{field_path}'.")
    output: list[str] = []
    for index, item in enumerate(value):
        if not isinstance(item, str) or not item.strip():
            raise ValueError(f"Expected non-empty string at '{field_path}[{index}]'.")
        output.append(normalize_rel_path(item))
    return output
