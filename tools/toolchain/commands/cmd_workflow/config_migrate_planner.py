import tomllib
from pathlib import Path, PurePosixPath
from typing import Any

from . import config_migrate_paths


def normalize_path_value(raw_value: str) -> str:
    value = raw_value.strip().replace("\\", "/")
    while value.startswith("./"):
        value = value[2:]
    if value.startswith("config/"):
        value = value[len("config/") :]
    value = str(PurePosixPath(value))
    if value in {"", "."}:
        raise RuntimeError("path must be a non-empty string.")
    return value


def require_table(
    parent: dict[str, Any], key: str, source_path: Path, field_prefix: str
) -> dict[str, Any]:
    value = parent.get(key)
    field_path = f"{field_prefix}.{key}" if field_prefix else key
    if value is None:
        raise RuntimeError(
            f"Invalid config [{source_path}] field '{field_path}': is required and must be a table."
        )
    if not isinstance(value, dict):
        raise RuntimeError(f"Invalid config [{source_path}] field '{field_path}': must be a table.")
    return value


def find_non_empty_string_alias(
    table: dict[str, Any],
    aliases: tuple[str, ...],
    source_path: Path,
    field_prefix: str,
) -> tuple[str, str]:
    for alias in aliases:
        if alias not in table:
            continue
        value = table[alias]
        field_path = f"{field_prefix}.{alias}" if field_prefix else alias
        if not isinstance(value, str):
            raise RuntimeError(
                f"Invalid config [{source_path}] field '{field_path}': must be a string."
            )
        if not value.strip():
            raise RuntimeError(
                f"Invalid config [{source_path}] field '{field_path}': must be a non-empty string."
            )
        return alias, value

    raise RuntimeError(
        f"Invalid config [{source_path}] field '{field_prefix}': "
        f"is required. Supported keys: {', '.join(aliases)}."
    )


def parse_toml_file(path: Path) -> dict[str, Any]:
    try:
        with path.open("rb") as handle:
            data = tomllib.load(handle)
    except tomllib.TOMLDecodeError as exc:
        raise RuntimeError(f"Failed to parse TOML [{path}]: {exc}") from exc
    except OSError as exc:
        raise RuntimeError(f"Failed to read TOML [{path}]: {exc}") from exc
    if not isinstance(data, dict):
        raise RuntimeError(f"Invalid TOML root [{path}]: must be a table.")
    return data


def extract_converter_interval_path(config_toml: dict[str, Any], source_path: Path) -> str:
    converter_tbl = require_table(config_toml, "converter", source_path, "")
    _, interval_path = find_non_empty_string_alias(
        converter_tbl,
        (
            "interval_config",
            "interval_processor_config",
            "interval_processor_config_path",
        ),
        source_path,
        "converter",
    )
    return normalize_path_value(interval_path)


def extract_report_paths(
    config_toml: dict[str, Any], source_path: Path
) -> dict[str, dict[str, str]]:
    reports_tbl = require_table(config_toml, "reports", source_path, "")
    alias_map: list[tuple[str, tuple[str, ...]]] = [
        ("markdown", ("markdown", "md")),
        ("latex", ("latex", "tex")),
        ("typst", ("typst", "typ")),
    ]

    result: dict[str, dict[str, str]] = {}
    for canonical_name, aliases in alias_map:
        selected_alias: str | None = None
        selected_tbl: dict[str, Any] | None = None
        for alias in aliases:
            if alias not in reports_tbl:
                continue
            candidate = reports_tbl[alias]
            field_path = f"reports.{alias}"
            if not isinstance(candidate, dict):
                raise RuntimeError(
                    f"Invalid config [{source_path}] field '{field_path}': must be a table."
                )
            selected_alias = alias
            selected_tbl = candidate
            break

        if selected_tbl is None or selected_alias is None:
            continue

        section_prefix = f"reports.{selected_alias}"
        _, day = find_non_empty_string_alias(selected_tbl, ("day",), source_path, section_prefix)
        _, month = find_non_empty_string_alias(
            selected_tbl,
            ("month",),
            source_path,
            section_prefix,
        )
        _, period = find_non_empty_string_alias(
            selected_tbl,
            ("period", "range"),
            source_path,
            section_prefix,
        )
        _, week = find_non_empty_string_alias(selected_tbl, ("week",), source_path, section_prefix)
        _, year = find_non_empty_string_alias(selected_tbl, ("year",), source_path, section_prefix)

        result[canonical_name] = {
            "day": normalize_path_value(day),
            "month": normalize_path_value(month),
            "period": normalize_path_value(period),
            "week": normalize_path_value(week),
            "year": normalize_path_value(year),
        }

    if not result:
        raise RuntimeError(
            f"Invalid config [{source_path}] field 'reports': "
            "must contain at least one format table (markdown/md, latex/tex, typst/typ)."
        )
    return result


def extract_converter_companion_files(config_root: Path, interval_rel_path: str) -> list[str]:
    interval_abs = config_migrate_paths.to_absolute_path(config_root, interval_rel_path)
    if not interval_abs.exists():
        return []

    interval_toml = parse_toml_file(interval_abs)
    companions: list[str] = []

    if "mappings_config_path" in interval_toml:
        raise RuntimeError(
            f"Invalid config [{interval_abs}] field 'mappings_config_path': "
            "legacy key is no longer supported; use 'alias_mapping_path'."
        )

    for key in ("alias_mapping_path", "duration_rules_config_path"):
        raw_value = interval_toml.get(key)
        if not isinstance(raw_value, str) or not raw_value.strip():
            continue
        normalized = normalize_path_value(raw_value)
        absolute_path = config_migrate_paths.to_absolute_path(interval_abs.parent, normalized)
        companions.append(config_migrate_paths.to_bundle_relative_path(config_root, absolute_path))

    return companions


def collect_optional_files(config_root: Path) -> list[str]:
    candidates = [
        config_root / "reports" / "latex" / "common_style.toml",
        config_root / "reports" / "typst" / "common_style.toml",
    ]
    result: list[str] = []
    for path in candidates:
        if path.exists():
            result.append(path.relative_to(config_root).as_posix())
    return result


def build_bundle_model(
    config_root: Path,
    source_config_path: Path,
    profile: str,
) -> dict[str, Any]:
    config_toml = parse_toml_file(source_config_path)
    interval_rel_path = extract_converter_interval_path(config_toml, source_config_path)
    report_paths = extract_report_paths(config_toml, source_config_path)

    required: set[str] = {"config.toml", interval_rel_path}
    for report in report_paths.values():
        required.update(report.values())
    required.update(extract_converter_companion_files(config_root, interval_rel_path))

    return {
        "schema_version": 1,
        "profile": profile,
        "bundle_name": "tracer_core_config",
        "file_list": {
            "required": sorted(required),
            "optional": collect_optional_files(config_root),
        },
        "paths": {
            "converter": {"interval_config": interval_rel_path},
            "reports": report_paths,
        },
    }
