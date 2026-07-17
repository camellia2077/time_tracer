from __future__ import annotations

import argparse
import shutil
from pathlib import Path


CUSTOM_ALIAS = "cliimportalias"
CUSTOM_PARENT = "zzdemo"
CUSTOM_LEAF = "only"
CUSTOM_CHILD_FILE = "zzdemo.toml"

# This fixture intentionally creates a small custom package for config-refresh
# regression tests. When imported via exchange replace-all, runtime DB is
# expected to be cleared/rebuilt from this custom source (not the baseline set).


def _backup_converter_config(workspace_root: Path, scenario_root: Path) -> None:
    converter_root = workspace_root / "config" / "converter"
    backup_root = scenario_root / "backup_config"
    backup_root.mkdir(parents=True, exist_ok=True)
    for file_name in (
        "interval_processor_config.toml",
        "alias_mapping.toml",
    ):
        shutil.copy2(converter_root / file_name, backup_root / file_name)
    aliases_root = converter_root / "aliases"
    backup_aliases_root = backup_root / "aliases"
    if backup_aliases_root.exists():
        shutil.rmtree(backup_aliases_root)
    shutil.copytree(aliases_root, backup_aliases_root)


def _append_custom_alias(alias_mapping_path: Path) -> None:
    content = alias_mapping_path.read_text(encoding="utf-8")
    custom_include = f'"aliases/{CUSTOM_CHILD_FILE}"'
    if custom_include in content:
        return

    suffix = "" if content.endswith("\n") else "\n"
    alias_mapping_path.write_text(
        content.replace(
            "]",
            f'  "aliases/{CUSTOM_CHILD_FILE}",\n]',
            1,
        )
        + suffix,
        encoding="utf-8",
    )


def _write_custom_alias_child(converter_root: Path) -> None:
    custom_child_path = converter_root / "aliases" / CUSTOM_CHILD_FILE
    custom_child_path.parent.mkdir(parents=True, exist_ok=True)
    custom_child_path.write_text(
        'parent = "zzdemo"\n\n[aliases]\n"cliimportalias" = "only"\n',
        encoding="utf-8",
    )


def _build_custom_txt(target_txt_path: Path) -> None:
    # Keep this regression fixture independent from the full canonical dataset:
    # the test only needs one valid activity to prove the imported alias config
    # is applied during the replace-all rebuild.
    content = "y2026\nm03\n\nd0301\n0600wake\n0610cliimportalias\n"
    target_txt_path.parent.mkdir(parents=True, exist_ok=True)
    target_txt_path.write_text(content, encoding="utf-8")


def prepare_fixture(workspace_root: Path, scenario_root: Path) -> None:
    _backup_converter_config(workspace_root, scenario_root)
    converter_root = workspace_root / "config" / "converter"
    _append_custom_alias(converter_root / "alias_mapping.toml")
    _write_custom_alias_child(converter_root)

    custom_data_root = scenario_root / "custom_data"
    target_txt_path = custom_data_root / "2026" / "2026-03.txt"
    _build_custom_txt(target_txt_path)

    print(f"[PASS] prepared exchange config refresh fixture at {scenario_root}")


def restore_runtime(workspace_root: Path, scenario_root: Path) -> None:
    converter_root = workspace_root / "config" / "converter"
    backup_root = scenario_root / "backup_config"
    if not backup_root.is_dir():
        raise RuntimeError(f"backup config root is missing: {backup_root}")

    for file_name in (
        "interval_processor_config.toml",
        "alias_mapping.toml",
    ):
        shutil.copy2(backup_root / file_name, converter_root / file_name)
    aliases_root = converter_root / "aliases"
    if aliases_root.exists():
        shutil.rmtree(aliases_root)
    shutil.copytree(backup_root / "aliases", aliases_root)

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
        )
        return 0

    if args.action == "restore":
        restore_runtime(workspace_root=workspace_root, scenario_root=scenario_root)
        return 0

    raise RuntimeError(f"unsupported action: {args.action}")


if __name__ == "__main__":
    raise SystemExit(main())
