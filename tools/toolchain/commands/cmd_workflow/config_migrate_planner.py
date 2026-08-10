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


def extract_converter_main_config_path(config_toml: dict[str, Any], source_path: Path) -> str:
    converter_tbl = require_table(config_toml, "converter", source_path, "")
    _, main_config_path = find_non_empty_string_alias(
        converter_tbl,
        ("main_config",),
        source_path,
        "converter",
    )
    return normalize_path_value(main_config_path)


def extract_insights_paths(
    config_toml: dict[str, Any], source_path: Path
) -> dict[str, dict[str, Any]]:
    if "reports" in config_toml:
        raise RuntimeError(
            f"Unknown config table [{source_path}]: 'reports'."
        )

    insights_tbl = require_table(config_toml, "insights", source_path, "")
    formats = ("markdown", "latex", "typst")

    result: dict[str, dict[str, Any]] = {}
    for format_name in formats:
        if format_name not in insights_tbl:
            continue
        selected_tbl = insights_tbl[format_name]
        field_path = f"insights.{format_name}"
        if not isinstance(selected_tbl, dict):
            raise RuntimeError(
                f"Invalid config [{source_path}] field '{field_path}': must be a table."
            )

        section: dict[str, Any] = {
            "root": normalize_path_value(
                find_non_empty_string_alias(
                    selected_tbl,
                    ("root",),
                    source_path,
                    field_path,
                )[1]
            )
        }
        if format_name == "markdown":
            _, default_locale = find_non_empty_string_alias(
                selected_tbl, ("default_locale",), source_path, field_path
            )
            supported = selected_tbl.get("supported_locales")
            if not isinstance(supported, list) or not all(
                isinstance(locale, str) and locale for locale in supported
            ):
                raise RuntimeError(
                    f"Invalid config [{source_path}] field '{field_path}.supported_locales': "
                    "must be a non-empty string array."
                )
            section["default_locale"] = default_locale
            section["supported_locales"] = list(supported)
        result[format_name] = section

    if not result:
        raise RuntimeError(
            f"Invalid config [{source_path}] field 'insights': "
            "must contain at least one format table (markdown, latex, typst)."
        )
    return result


def collect_optional_files(config_root: Path) -> list[str]:
    candidates = [
        config_root / "insights" / "latex" / "common_style.toml",
        config_root / "insights" / "typst" / "common_style.toml",
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
    main_config_rel_path = extract_converter_main_config_path(config_toml, source_config_path)
    insights_paths = extract_insights_paths(config_toml, source_config_path)

    required: set[str] = {"config.toml", main_config_rel_path}
    for format_name, insights in insights_paths.items():
        root = config_root / str(insights["root"])
        if format_name == "markdown":
            locales = [str(locale) for locale in insights["supported_locales"]]
            for locale in locales:
                required.update(
                    str(path.relative_to(config_root)).replace("\\", "/")
                    for path in (root / locale).glob("*.toml")
                )
        else:
            required.update(
                str(path.relative_to(config_root)).replace("\\", "/")
                for path in root.glob("*.toml")
            )

    optional = set(collect_optional_files(config_root))
    for path in config_root.rglob("*.toml"):
        relative = path.relative_to(config_root).as_posix()
        if relative in optional or relative == "meta/bundle.toml":
            continue
        required.add(relative)
    required.difference_update(optional)

    return {
        "schema_version": 1,
        "profile": profile,
        "bundle_name": "tracer_core_config",
        "file_list": {
            "required": sorted(required),
            "optional": sorted(optional),
        },
    }
