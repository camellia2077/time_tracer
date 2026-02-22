from pathlib import Path
from typing import Any

from .schema_common import (
    _append_error,
    _as_list,
    _require_bool_like,
    _require_non_empty_string,
    _require_table,
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


def _validate_log_routing(
    toml_data: dict[str, Any],
    errors: list[str],
) -> None:
    log_routing = toml_data.get("log_routing")
    if log_routing is None:
        return
    if not isinstance(log_routing, dict):
        _append_error(errors, "log_routing", "must be a table.")
        return

    rules = log_routing.get("rules", [])
    if not isinstance(rules, list):
        _append_error(errors, "log_routing.rules", "must be an array of tables.")
        return

    for index, rule in enumerate(rules):
        rule_path = f"log_routing.rules[{index}]"
        if not isinstance(rule, dict):
            _append_error(errors, rule_path, "must be a table.")
            continue

        _require_non_empty_string(rule, "stage", rule_path, errors)
        _require_non_empty_string(rule, "subdir", rule_path, errors)
        for key in ("command_prefix", "legacy_name_contains"):
            values = _as_list(rule.get(key))
            for value_index, value in enumerate(values):
                if not isinstance(value, str) or not value.strip():
                    _append_error(
                        errors,
                        f"{rule_path}.{key}[{value_index}]",
                        "must be a non-empty string.",
                    )
