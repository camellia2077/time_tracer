from typing import Any

from .schema_common import (
    IDENTIFIER_PATTERN,
    RUNTIME_VARIABLES,
    _append_error,
    _as_list,
    _require_non_empty_string,
    _validate_placeholder_usage,
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

        if "raw_command" in command and not isinstance(command.get("raw_command"), bool):
            _append_error(errors, f"{command_path}.raw_command", "must be a bool.")

        strings_to_validate: list[str] = []
        for key in ("name", "stage", "stdin_input"):
            value = command.get(key)
            if isinstance(value, str):
                strings_to_validate.append(value)
        strings_to_validate.extend([str(item) for item in args_list])
        for key in (
            "expect_files",
            "expect_file_contains",
            "expect_stdout_contains",
            "expect_stderr_contains",
        ):
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

        if "raw_command" in group and not isinstance(group.get("raw_command"), bool):
            _append_error(errors, f"{group_path}.raw_command", "must be a bool.")

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
        for key in (
            "expect_files",
            "expect_file_contains",
            "expect_stdout_contains",
            "expect_stderr_contains",
        ):
            strings_to_validate.extend([str(item) for item in _as_list(group.get(key))])

        _validate_placeholder_usage(
            values=strings_to_validate,
            allowed=allowed_placeholders,
            path=group_path,
            errors=errors,
        )
