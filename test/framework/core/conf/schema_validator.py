from pathlib import Path
from typing import Any

from .schema_commands import _validate_commands
from .schema_common import _validate_no_unresolved_dollar_variables
from .schema_sections import (
    _validate_cli_names,
    _validate_log_routing,
    _validate_paths,
    _validate_pipeline,
    _validate_required_sections,
    _validate_run_control,
)


def validate_suite_schema(
    toml_data: dict[str, Any],
    config_path: Path,
) -> None:
    errors: list[str] = []

    paths, cli_names, run_control, pipeline = _validate_required_sections(toml_data, errors)
    _validate_no_unresolved_dollar_variables(toml_data, "", errors)
    _validate_paths(paths, config_path, errors)
    _validate_cli_names(cli_names, errors)
    _validate_run_control(run_control, errors)
    _validate_pipeline(pipeline, errors)
    _validate_commands(toml_data, errors)
    _validate_log_routing(toml_data, errors)

    if errors:
        details = "\n".join(f" - {error}" for error in errors)
        raise ValueError(f"Suite schema validation failed:\n{details}")
