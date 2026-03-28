from __future__ import annotations

import argparse
import shutil
from pathlib import Path


CUSTOM_ALIAS = "cliimportalias"
CUSTOM_PROJECT_PATH = "zzdemo_only"


def _backup_converter_config(workspace_root: Path, scenario_root: Path) -> None:
    converter_root = workspace_root / "config" / "converter"
    backup_root = scenario_root / "backup_config"
    backup_root.mkdir(parents=True, exist_ok=True)
    for file_name in (
        "interval_processor_config.toml",
        "alias_mapping.toml",
        "duration_rules.toml",
    ):
        shutil.copy2(converter_root / file_name, backup_root / file_name)


def _append_custom_alias(alias_mapping_path: Path) -> None:
    content = alias_mapping_path.read_text(encoding="utf-8")
    custom_line = f'"{CUSTOM_ALIAS}" = "{CUSTOM_PROJECT_PATH}"'
    if custom_line in content:
        return

    suffix = "" if content.endswith("\n") else "\n"
    alias_mapping_path.write_text(
        f"{content}{suffix}\n{custom_line}\n",
        encoding="utf-8",
    )


def _build_custom_txt(source_txt_path: Path, target_txt_path: Path) -> None:
    lines = source_txt_path.read_text(encoding="utf-8").splitlines()
    if any(CUSTOM_ALIAS in line for line in lines):
        raise RuntimeError(f"source TXT already contains {CUSTOM_ALIAS}")

    try:
        anchor = lines.index("0205meal")
    except ValueError as error:
        raise RuntimeError("expected activity line `0205meal` in source TXT") from error

    lines.insert(anchor + 1, f"0210{CUSTOM_ALIAS}")
    target_txt_path.parent.mkdir(parents=True, exist_ok=True)
    target_txt_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def prepare_fixture(workspace_root: Path, scenario_root: Path, source_txt_path: Path) -> None:
    _backup_converter_config(workspace_root, scenario_root)
    _append_custom_alias(workspace_root / "config" / "converter" / "alias_mapping.toml")

    custom_data_root = scenario_root / "custom_data"
    target_txt_path = custom_data_root / "2026" / "2026-03.txt"
    _build_custom_txt(source_txt_path, target_txt_path)

    print(f"[PASS] prepared exchange config refresh fixture at {scenario_root}")


def restore_runtime(workspace_root: Path, scenario_root: Path) -> None:
    converter_root = workspace_root / "config" / "converter"
    backup_root = scenario_root / "backup_config"
    if not backup_root.is_dir():
        raise RuntimeError(f"backup config root is missing: {backup_root}")

    for file_name in (
        "interval_processor_config.toml",
        "alias_mapping.toml",
        "duration_rules.toml",
    ):
        shutil.copy2(backup_root / file_name, converter_root / file_name)

    input_root = workspace_root / "input" / "full"
    if input_root.exists():
        shutil.rmtree(input_root)
    input_root.mkdir(parents=True, exist_ok=True)

    db_root = workspace_root / "output" / "db"
    db_root.mkdir(parents=True, exist_ok=True)
    for candidate in db_root.glob("time_data.sqlite3*"):
        if candidate.is_dir():
            shutil.rmtree(candidate)
        else:
            candidate.unlink()

    print(f"[PASS] restored stale runtime config and cleared runtime state at {workspace_root}")


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Prepare or restore the Windows CLI exchange config refresh regression fixture."
    )
    subparsers = parser.add_subparsers(dest="action", required=True)

    prepare_parser = subparsers.add_parser("prepare")
    prepare_parser.add_argument("--workspace-root", required=True)
    prepare_parser.add_argument("--scenario-root", required=True)
    prepare_parser.add_argument("--source-txt", required=True)

    restore_parser = subparsers.add_parser("restore")
    restore_parser.add_argument("--workspace-root", required=True)
    restore_parser.add_argument("--scenario-root", required=True)

    args = parser.parse_args()
    workspace_root = Path(args.workspace_root).resolve()
    scenario_root = Path(args.scenario_root).resolve()

    if args.action == "prepare":
        prepare_fixture(
            workspace_root=workspace_root,
            scenario_root=scenario_root,
            source_txt_path=Path(args.source_txt).resolve(),
        )
        return 0

    if args.action == "restore":
        restore_runtime(workspace_root=workspace_root, scenario_root=scenario_root)
        return 0

    raise RuntimeError(f"unsupported action: {args.action}")


if __name__ == "__main__":
    raise SystemExit(main())
