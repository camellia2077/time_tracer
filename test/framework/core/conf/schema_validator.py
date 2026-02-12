import re
from pathlib import Path
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


def _validate_required_sections(
    toml_data: dict[str, Any],
    errors: list[str],
) -> tuple[dict[str, Any], dict[str, Any], dict[str, Any], dict[str, Any]]:
    paths = _require_table(toml_data, "paths", errors)
    cli_names = _require_table(toml_data, "cli_names", errors)
    run_control = _require_table(toml_data, "run_control", errors)
    pipeline = _require_table(toml_data, "pipeline", errors)
    return paths, cli_names, run_control, pipeline


def _validate_paths(
    paths: dict[str, Any],
    config_path: Path,
    errors: list[str],
) -> None:
    required_fields = [
        "project_apps_root",
        "test_data_path",
        "target_executables_dir",
        "output_dir_name",
        "py_output_dir",
    ]
    for field in required_fields:
        _require_non_empty_string(paths, field, "paths", errors)

    if "default_build_dir" in paths:
        _require_non_empty_string(paths, "default_build_dir", "paths", errors)

    project_apps_root = _require_non_empty_string(
        paths, "project_apps_root", "paths", errors
    )
    test_data_path = _require_non_empty_string(paths, "test_data_path", "paths", errors)

    must_exist_map = {
        "paths.project_apps_root": project_apps_root,
        "paths.test_data_path": test_data_path,
    }
    for field_path, field_value in must_exist_map.items():
        if not field_value:
            continue
        path_obj = Path(field_value)
        if not path_obj.is_absolute():
            path_obj = (config_path.parent / path_obj).resolve()
        if not path_obj.exists():
            _append_error(errors, field_path, f"path does not exist: `{path_obj}`.")


def _validate_cli_names(cli_names: dict[str, Any], errors: list[str]) -> None:
    _require_non_empty_string(cli_names, "executable_cli_name", "cli_names", errors)
    _require_non_empty_string(cli_names, "executable_app_name", "cli_names", errors)


def _validate_run_control(run_control: dict[str, Any], errors: list[str]) -> None:
    required_keys = [
        "enable_environment_clean",
        "enable_environment_prepare",
        "enable_test_execution",
        "stop_on_failure",
    ]
    for key in required_keys:
        if key not in run_control:
            _append_error(errors, f"run_control.{key}", "is required.")
            continue
        _require_bool_like(run_control, key, "run_control", errors)


def _validate_pipeline(pipeline: dict[str, Any], errors: list[str]) -> None:
    mode = _require_non_empty_string(pipeline, "mode", "pipeline", errors)
    if mode is None:
        return
    if mode.strip().lower() not in {"ingest", "staged", "none"}:
        _append_error(
            errors,
            "pipeline.mode",
            "must be one of: ingest, staged, none.",
        )


def _validate_commands(
    toml_data: dict[str, Any],
    errors: list[str],
) -> None:
    commands = toml_data.get("commands", [])
    command_groups = toml_data.get("command_groups", [])

    if not isinstance(commands, list):
        _append_error(errors, "commands", "must be an array of tables.")
        commands = []
    if not isinstance(command_groups, list):
        _append_error(errors, "command_groups", "must be an array of tables.")
        command_groups = []

    if not commands and not command_groups:
        _append_error(
            errors,
            "commands/command_groups",
            "at least one command or command_group must be defined.",
        )

    for index, command in enumerate(commands):
        command_path = f"commands[{index}]"
        if not isinstance(command, dict):
            _append_error(errors, command_path, "must be a table.")
            continue

        _require_non_empty_string(command, "name", command_path, errors)
        args = command.get("args")
        if args is None:
            _append_error(errors, f"{command_path}.args", "is required.")
            args_list: list[Any] = []
        else:
            args_list = _as_list(args)
            if not args_list:
                _append_error(errors, f"{command_path}.args", "must be non-empty.")

        strings_to_validate: list[str] = []
        for key in ("name", "stage", "stdin_input"):
            value = command.get(key)
            if isinstance(value, str):
                strings_to_validate.append(value)
        strings_to_validate.extend([str(item) for item in args_list])
        for key in ("expect_files", "expect_stdout_contains", "expect_stderr_contains"):
            strings_to_validate.extend([str(item) for item in _as_list(command.get(key))])

        _validate_placeholder_usage(
            values=strings_to_validate,
            allowed=RUNTIME_VARIABLES,
            path=command_path,
            errors=errors,
        )

    for index, group in enumerate(command_groups):
        group_path = f"command_groups[{index}]"
        if not isinstance(group, dict):
            _append_error(errors, group_path, "must be a table.")
            continue

        _require_non_empty_string(group, "name", group_path, errors)
        template = group.get("template", group.get("args"))
        template_list = _as_list(template)
        if not template_list:
            _append_error(errors, f"{group_path}.template", "is required and must be non-empty.")

        matrix = group.get("matrix", {})
        if matrix is None:
            matrix = {}
        if not isinstance(matrix, dict):
            _append_error(errors, f"{group_path}.matrix", "must be a table.")
            matrix = {}

        matrix_keys: set[str] = set()
        for matrix_key, matrix_value in matrix.items():
            if not isinstance(matrix_key, str) or not IDENTIFIER_PATTERN.match(matrix_key):
                _append_error(
                    errors,
                    f"{group_path}.matrix.{matrix_key}",
                    "matrix key must match identifier pattern.",
                )
            matrix_values = _as_list(matrix_value)
            if not matrix_values:
                _append_error(
                    errors,
                    f"{group_path}.matrix.{matrix_key}",
                    "matrix value list must be non-empty.",
                )
            matrix_keys.add(str(matrix_key))

        allowed_placeholders = set(RUNTIME_VARIABLES) | matrix_keys
        strings_to_validate = [str(item) for item in template_list]
        for key in (
            "name_template",
            "stage",
            "stdin_input",
        ):
            value = group.get(key)
            if isinstance(value, str):
                strings_to_validate.append(value)
        for key in ("expect_files", "expect_stdout_contains", "expect_stderr_contains"):
            strings_to_validate.extend([str(item) for item in _as_list(group.get(key))])

        _validate_placeholder_usage(
            values=strings_to_validate,
            allowed=allowed_placeholders,
            path=group_path,
            errors=errors,
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


def validate_suite_schema(
    toml_data: dict[str, Any],
    config_path: Path,
) -> None:
    errors: list[str] = []

    paths, cli_names, run_control, pipeline = _validate_required_sections(
        toml_data, errors
    )
    _validate_no_unresolved_dollar_variables(toml_data, "", errors)
    _validate_paths(paths, config_path, errors)
    _validate_cli_names(cli_names, errors)
    _validate_run_control(run_control, errors)
    _validate_pipeline(pipeline, errors)
    _validate_commands(toml_data, errors)

    if errors:
        details = "\n".join(f" - {error}" for error in errors)
        raise ValueError(f"Suite schema validation failed:\n{details}")
