from __future__ import annotations

import difflib
import tomllib
from pathlib import Path

from .bundle import build_bundle_model, load_source_bundle
from .constants import SUPPORTED_TARGETS
from .files import apply_plan, collect_plan_files, read_existing_files


def print_bundle_diff(output_root: Path, generated_bundle: str) -> None:
    bundle_path = output_root / "meta" / "bundle.toml"
    existing_text = bundle_path.read_text(encoding="utf-8") if bundle_path.exists() else ""
    diff_lines = list(
        difflib.unified_diff(
            existing_text.splitlines(),
            generated_bundle.splitlines(),
            fromfile=f"{bundle_path} (current)",
            tofile=f"{bundle_path} (generated)",
            lineterm="",
        )
    )
    if diff_lines:
        print("\n".join(diff_lines))
    else:
        print("--- bundle.toml is already up to date.")


def sync_target(
    *,
    target: str,
    source_root: Path,
    output_root: Path,
    apply: bool,
    show_diff: bool,
    allow_overwrite_source: bool,
) -> None:
    resolved_source = source_root.resolve()
    resolved_output = output_root.resolve()
    if resolved_source == resolved_output and not allow_overwrite_source:
        raise ValueError(
            "Refusing to overwrite source config in-place. "
            "Use --allow-overwrite-source to override."
        )

    source_bundle = load_source_bundle(source_root)
    model = build_bundle_model(source_bundle, target)
    planned_files = collect_plan_files(source_root, model)
    existing_files = read_existing_files(output_root)

    planned_keys = set(planned_files)
    existing_keys = set(existing_files)
    added = sorted(planned_keys - existing_keys)
    removed = sorted(existing_keys - planned_keys)
    changed = sorted(
        rel for rel in (planned_keys & existing_keys) if planned_files[rel] != existing_files[rel]
    )

    print(f"=== target: {target}")
    print(f"source: {source_root}")
    print(f"output: {output_root}")
    print(f"planned files: {len(planned_files)}")
    print(f"changes: +{len(added)} ~{len(changed)} -{len(removed)}")

    if show_diff:
        generated_bundle = planned_files["meta/bundle.toml"].decode("utf-8")
        print_bundle_diff(output_root, generated_bundle)

    if not apply:
        print("--- dry-run: no files were written.")
        return

    apply_plan(output_root, planned_files, removed)
    print("--- apply: synchronized platform config.")


def run_generation(
    *,
    target: str,
    source_root: Path,
    windows_output_root: Path,
    android_output_root: Path,
    apply: bool,
    show_diff: bool,
    allow_overwrite_source: bool,
) -> int:
    if not source_root.exists():
        print(f"Source root does not exist: {source_root}")
        return 1

    targets: list[str] = list(SUPPORTED_TARGETS) if target == "both" else [target]
    try:
        for item in targets:
            output_root = (
                windows_output_root.resolve()
                if item == "windows"
                else android_output_root.resolve()
            )
            sync_target(
                target=item,
                source_root=source_root.resolve(),
                output_root=output_root,
                apply=apply,
                show_diff=show_diff,
                allow_overwrite_source=allow_overwrite_source,
            )
    except (FileNotFoundError, ValueError, tomllib.TOMLDecodeError) as error:
        print(str(error))
        return 1
    return 0
