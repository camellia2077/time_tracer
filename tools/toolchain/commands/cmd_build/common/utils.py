from pathlib import Path


def normalize_cache_path(path_value: str) -> str:
    return path_value.strip().replace("\\", "/").rstrip("/")


def to_absolute_path(path_str: str) -> Path:
    return Path(path_str).resolve()
