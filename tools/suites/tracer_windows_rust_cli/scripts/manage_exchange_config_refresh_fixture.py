from __future__ import annotations

import argparse
import re
import shutil
from pathlib import Path


CUSTOM_ALIAS = "cliimportalias"
CUSTOM_PARENT = "zzdemo"
CUSTOM_LEAF = "only"
CUSTOM_CHILD_FILE = "zzdemo.toml"
EVENT_RE = re.compile(r"^(?P<start>\d{4})(?:-(?P<end>\d{4}))?")

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
        "duration_rules.toml",
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


def _build_custom_txt(source_txt_path: Path, target_txt_path: Path) -> None:
    # Design intent:
    # build a deterministic config-refresh fixture by injecting one synthetic line
    # into 2026-03 data so exchange import can verify both:
    # 1) replace-all DB rebuild from fixture payload, and
    # 2) alias mapping refresh (`cliimportalias` -> `zzdemo_only`).
    #
    # Concrete mutation:
    # insert `0210cliimportalias` immediately after the first authored `meal`
    # line. The generated fixture may be point-based (`0205meal`) or interval-
    # based (`0055-0155meal`); this helper accepts either authored shape so the
    # exchange refresh scenario stays coupled to current canonical test data.
    lines = source_txt_path.read_text(encoding="utf-8").splitlines()
    if any(CUSTOM_ALIAS in line for line in lines):
        raise RuntimeError(f"source TXT already contains {CUSTOM_ALIAS}")

    meal_anchor = next(
        (index for index, line in enumerate(lines) if line.endswith("meal")),
        None,
    )
    if meal_anchor is None:
        raise RuntimeError("expected an authored `meal` activity line in source TXT")

    day_end = next(
        (index for index in range(meal_anchor + 1, len(lines)) if lines[index] == ""),
        len(lines),
    )
    activity_lines = [
        line for line in lines[meal_anchor + 1 : day_end] if EVENT_RE.match(line)
    ]
    if not activity_lines:
        raise RuntimeError("expected additional activity lines after the `meal` anchor")

    last_event_match = EVENT_RE.match(activity_lines[-1])
    if last_event_match is None:
        raise RuntimeError("failed to parse the last activity line in meal day block")
    last_end = last_event_match.group("end") or last_event_match.group("start")
    if last_end is None:
        raise RuntimeError("failed to resolve last event end time")

    hours = int(last_end[:2])
    minutes = int(last_end[2:])
    custom_minutes = (hours * 60 + minutes + 1) % (24 * 60)
    custom_time = f"{custom_minutes // 60:02d}{custom_minutes % 60:02d}"

    lines.insert(day_end, f"{custom_time}{CUSTOM_ALIAS}")
    target_txt_path.parent.mkdir(parents=True, exist_ok=True)
    target_txt_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def prepare_fixture(workspace_root: Path, scenario_root: Path, source_txt_path: Path) -> None:
    _backup_converter_config(workspace_root, scenario_root)
    converter_root = workspace_root / "config" / "converter"
    _append_custom_alias(converter_root / "alias_mapping.toml")
    _write_custom_alias_child(converter_root)

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
