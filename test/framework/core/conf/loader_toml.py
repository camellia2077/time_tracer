import re
import sys
from pathlib import Path
from typing import Any

# Python 3.11+ 内置 tomllib
if sys.version_info >= (3, 11):
    import tomllib
else:
    try:
        import tomli as tomllib
    except ImportError:
        raise ImportError(
            "For Python versions < 3.11, please install 'tomli' using 'pip install tomli'"
        )


_VAR_PATTERN = re.compile(r"\$\{([a-zA-Z_][a-zA-Z0-9_]*)\}")
_PATH_KEYS = {
    "project_apps_root",
    "source_executables_dir",
    "test_data_path",
    "target_executables_dir",
    "db_dir",
    "export_output_dir",
    "py_output_dir",
    "output_dir_name",
    "processed_json_dir",
}


def _merge_toml(base, override, key_path=""):
    if isinstance(base, dict) and isinstance(override, dict):
        merged = dict(base)
        for key, value in override.items():
            new_path = f"{key_path}.{key}" if key_path else key
            if key in merged:
                merged[key] = _merge_toml(merged[key], value, new_path)
            else:
                merged[key] = value
        return merged

    if isinstance(base, list) and isinstance(override, list):
        if key_path in {"commands", "command_groups"}:
            return base + override
        return override

    return override


def _detect_repo_root(config_path: Path) -> Path:
    resolved = config_path.resolve()
    for parent in [resolved.parent, *resolved.parents]:
        if (parent / "apps").exists() and (parent / "test").exists():
            return parent
    return resolved.parent


def _substitute_variables(value: str, variables: dict[str, str]) -> str:
    def replace(match):
        key = match.group(1)
        return str(variables.get(key, match.group(0)))

    return _VAR_PATTERN.sub(replace, value)


def _expand_variables(data: Any, variables: dict[str, str]) -> Any:
    if isinstance(data, dict):
        return {k: _expand_variables(v, variables) for k, v in data.items()}
    if isinstance(data, list):
        return [_expand_variables(item, variables) for item in data]
    if isinstance(data, str):
        return _substitute_variables(data, variables)
    return data


def _resolve_paths_section_for_file(data: dict, base_dir: Path) -> dict:
    paths_data = data.get("paths")
    if not isinstance(paths_data, dict):
        return data

    normalized_paths = dict(paths_data)
    for key, value in paths_data.items():
        if key not in _PATH_KEYS or not isinstance(value, str):
            continue
        value_str = value.strip()
        if not value_str:
            continue
        path_obj = Path(value_str)
        if not path_obj.is_absolute():
            path_obj = (base_dir / path_obj).resolve()
        normalized_paths[key] = str(path_obj)

    updated = dict(data)
    updated["paths"] = normalized_paths
    return updated


def _load_toml_with_includes(
    config_path: Path, visited=None, variables: dict[str, str] | None = None
) -> dict:
    if visited is None:
        visited = set()
    resolved = config_path.resolve()
    if resolved in visited:
        raise RuntimeError(f"Circular include detected at: {resolved}")
    visited.add(resolved)

    scoped_variables: dict[str, str] = dict(variables or {})
    scoped_variables["config_dir"] = str(resolved.parent)

    with open(resolved, "rb") as file:
        data = tomllib.load(file)
    data = _expand_variables(data, scoped_variables)

    merged = {}
    includes = data.get("includes", [])
    if isinstance(includes, list):
        for include in includes:
            include_path = Path(include)
            if not include_path.is_absolute():
                include_path = resolved.parent / include_path
            merged = _merge_toml(
                merged,
                _load_toml_with_includes(
                    include_path,
                    visited,
                    variables=scoped_variables,
                ),
            )

    data_no_includes = {k: v for k, v in data.items() if k != "includes"}
    data_no_includes = _resolve_paths_section_for_file(
        data_no_includes,
        resolved.parent,
    )
    merged = _merge_toml(merged, data_no_includes)
    return merged
