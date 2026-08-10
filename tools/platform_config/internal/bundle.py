from __future__ import annotations

import tomllib
from pathlib import Path
from typing import Any

from .constants import INSIGHTS_FORMAT_ORDER
from .model import BundleModel
from .path_utils import dedupe_keep_order, normalize_rel_path
from .validation import ensure_dict, ensure_list_of_str, ensure_str


def parse_insights_paths(
    insights_table: dict[str, Any], format_name: str, table_prefix: str
) -> dict[str, object]:
    format_table = ensure_dict(
        insights_table.get(format_name), f"{table_prefix}.{format_name}"
    )
    parsed: dict[str, object] = {
        "root": normalize_rel_path(
            ensure_str(format_table.get("root"), f"{table_prefix}.{format_name}.root")
        )
    }
    if format_name == "markdown":
        parsed["default_locale"] = ensure_str(
            format_table.get("default_locale"),
            f"{table_prefix}.{format_name}.default_locale",
        )
        supported_locales = format_table.get("supported_locales")
        if not isinstance(supported_locales, list) or not all(
            isinstance(locale, str) and locale for locale in supported_locales
        ):
            raise ValueError(
                f"{table_prefix}.{format_name}.supported_locales must be a non-empty string array"
            )
        parsed["supported_locales"] = list(supported_locales)
    return parsed


def load_source_bundle(source_root: Path) -> dict[str, Any]:
    bundle_path = source_root / "meta" / "bundle.toml"
    if not bundle_path.exists():
        raise FileNotFoundError(f"Source bundle not found: {bundle_path}")
    data = tomllib.loads(bundle_path.read_text(encoding="utf-8"))
    return ensure_dict(data, "root")


def load_source_config(source_root: Path) -> dict[str, Any]:
    config_path = source_root / "config.toml"
    if not config_path.exists():
        raise FileNotFoundError(f"Source config not found: {config_path}")
    data = tomllib.loads(config_path.read_text(encoding="utf-8"))
    return ensure_dict(data, "root")


def build_bundle_model(
    source_bundle: dict[str, Any],
    source_config: dict[str, Any],
    target: str,
    activity_hierarchy_files: list[str] | None = None,
) -> BundleModel:
    schema_version_raw = source_bundle.get("schema_version")
    if not isinstance(schema_version_raw, int):
        raise ValueError("Expected integer at 'schema_version'.")

    bundle_name = ensure_str(source_bundle.get("bundle_name"), "bundle_name")

    file_list = ensure_dict(source_bundle.get("file_list"), "file_list")
    source_required = ensure_list_of_str(file_list.get("required"), "file_list.required")
    source_optional = ensure_list_of_str(file_list.get("optional"), "file_list.optional")
    source_program_required = [
        path.removeprefix("program/") for path in source_required
    ]
    source_program_optional = [
        path.removeprefix("program/") for path in source_optional
    ]

    converter_table = ensure_dict(source_config.get("converter"), "converter")
    main_config = normalize_rel_path(
        ensure_str(converter_table.get("main_config"), "converter.main_config")
    )
    visualization_table = ensure_dict(
        source_config.get("visualization"), "visualization"
    )
    def program_path(value: object, field: str) -> str:
        return "program/" + normalize_rel_path(ensure_str(value, field))

    heatmap_config = program_path(
        visualization_table.get("heatmap"), "visualization.heatmap"
    )
    pie_config = program_path(
        visualization_table.get("pie"), "visualization.pie"
    )

    insights_table = ensure_dict(source_config.get("insights"), "insights")
    markdown_paths = parse_insights_paths(insights_table, "markdown", "insights")
    markdown_root = str(markdown_paths["root"])
    markdown_default_locale = str(markdown_paths["default_locale"])
    localized_markdown_files = [
        f"program/{path}"
        for path in source_program_required
        if path.startswith("insights/markdown/")
        and path.count("/") == 3
    ]

    if target == "android":
        required_files = dedupe_keep_order(
            [
                "program/config.toml",
                heatmap_config,
                pie_config,
                f"program/{markdown_root}/{markdown_default_locale}/day.toml",
                f"program/{markdown_root}/{markdown_default_locale}/month.toml",
                f"program/{markdown_root}/{markdown_default_locale}/period.toml",
                f"program/{markdown_root}/{markdown_default_locale}/week.toml",
                f"program/{markdown_root}/{markdown_default_locale}/year.toml",
                *localized_markdown_files,
            ]
        )
        return BundleModel(
            schema_version=schema_version_raw,
            profile="android",
            bundle_name=bundle_name,
            required_files=required_files,
            optional_files=[],
            converter_main_config="user/behavior.toml",
            visualization_heatmap_config=heatmap_config,
            insights={"markdown": markdown_paths},
        )

    windows_insights: dict[str, dict[str, object]] = {}
    for format_name in INSIGHTS_FORMAT_ORDER:
        if format_name in insights_table:
            windows_insights[format_name] = parse_insights_paths(
                insights_table, format_name, "insights"
            )
    if "markdown" not in windows_insights:
        raise ValueError("Windows bundle generation requires insights.markdown.")

    required_files = dedupe_keep_order(
        [
            "program/config.toml",
            heatmap_config,
            pie_config,
            *(f"program/{path}" for path in source_program_required),
        ]
    )
    optional_files = dedupe_keep_order(
        f"program/{path}" for path in source_program_optional
    )
    return BundleModel(
        schema_version=schema_version_raw,
        profile="windows",
        bundle_name=bundle_name,
        required_files=required_files,
        optional_files=optional_files,
        converter_main_config="user/behavior.toml",
        visualization_heatmap_config=heatmap_config,
        insights=windows_insights,
    )


def render_bundle_toml(model: BundleModel) -> str:
    lines: list[str] = [
        "# Auto-generated by tools/platform_config/run.py.",
        "# Source of truth: selected config/program",
        "",
        f"schema_version = {model.schema_version}",
        f'profile = "{model.profile}"',
        f'bundle_name = "{model.bundle_name}"',
        "",
        "[file_list]",
        "required = [",
    ]
    for path in model.required_files:
        lines.append(f'  "{path}",')
    lines.extend(["]", "optional = ["])
    for path in model.optional_files:
        lines.append(f'  "{path}",')
    lines.extend(["]", ""])
    return "\n".join(lines)


def build_android_config_toml(source_root: Path) -> str:
    source_config_path = source_root / "config.toml"
    text = source_config_path.read_text(encoding="utf-8")
    lines = text.splitlines(keepends=True)

    disallowed_tables = {"insights.typst", "insights.latex"}
    output_lines: list[str] = []
    skip_block = False
    for line in lines:
        stripped = line.strip()
        if stripped.startswith("[") and stripped.endswith("]"):
            table_name = stripped[1:-1].strip()
            skip_block = table_name in disallowed_tables
            if skip_block:
                continue
        if not skip_block:
            output_lines.append(line)

    output = "".join(output_lines)
    if output and not output.endswith("\n"):
        output += "\n"
    return output
