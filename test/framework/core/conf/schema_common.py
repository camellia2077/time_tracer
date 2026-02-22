import re
from typing import Any, Iterable


IDENTIFIER_PATTERN = re.compile(r"^[a-zA-Z_][a-zA-Z0-9_]*$")
BRACE_VAR_PATTERN = re.compile(r"(?<!\{)\{([a-zA-Z_][a-zA-Z0-9_]*)\}(?!\})")
DOLLAR_VAR_PATTERN = re.compile(r"\$\{([a-zA-Z_][a-zA-Z0-9_]*)\}")

RUNTIME_VARIABLES = {
    "data_path",
    "db_path",
    "output_dir",
    "export_output_dir",
    "exe_dir",
    "processed_json_path",
    "processed_json_dir",
}


def _as_list(value: Any) -> list[Any]:
    if value is None:
        return []
    if isinstance(value, list):
        return value
    return [value]


def _append_error(errors: list[str], path: str, message: str) -> None:
    errors.append(f"{path}: {message}")


def _require_table(
    data: dict[str, Any],
    key: str,
    errors: list[str],
) -> dict[str, Any]:
    value = data.get(key)
    if not isinstance(value, dict):
        _append_error(errors, key, "must be a table.")
        return {}
    return value


def _require_non_empty_string(
    table: dict[str, Any],
    key: str,
    path: str,
    errors: list[str],
) -> str | None:
    value = table.get(key)
    if not isinstance(value, str) or not value.strip():
        _append_error(errors, f"{path}.{key}", "must be a non-empty string.")
        return None
    return value


def _require_bool_like(
    table: dict[str, Any],
    key: str,
    path: str,
    errors: list[str],
) -> None:
    value = table.get(key)
    if isinstance(value, bool):
        return
    if isinstance(value, int) and value in (0, 1):
        return
    _append_error(errors, f"{path}.{key}", "must be bool or 0/1.")


def _extract_placeholders(value: str) -> set[str]:
    return set(BRACE_VAR_PATTERN.findall(value))


def _validate_placeholder_usage(
    values: Iterable[str],
    allowed: set[str],
    path: str,
    errors: list[str],
) -> None:
    for raw_value in values:
        if not isinstance(raw_value, str):
            _append_error(errors, path, "contains non-string placeholder template.")
            continue
        placeholders = _extract_placeholders(raw_value)
        for placeholder in sorted(placeholders):
            if placeholder not in allowed:
                _append_error(
                    errors,
                    path,
                    f"unknown placeholder `{{{placeholder}}}` in `{raw_value}`.",
                )


def _validate_no_unresolved_dollar_variables(
    node: Any,
    path: str,
    errors: list[str],
) -> None:
    if isinstance(node, dict):
        for key, value in node.items():
            next_path = f"{path}.{key}" if path else str(key)
            _validate_no_unresolved_dollar_variables(value, next_path, errors)
        return
    if isinstance(node, list):
        for index, item in enumerate(node):
            next_path = f"{path}[{index}]"
            _validate_no_unresolved_dollar_variables(item, next_path, errors)
        return
    if isinstance(node, str):
        unresolved = DOLLAR_VAR_PATTERN.findall(node)
        for unresolved_key in unresolved:
            _append_error(
                errors,
                path,
                f"unresolved variable `${{{unresolved_key}}}` in `{node}`.",
            )
