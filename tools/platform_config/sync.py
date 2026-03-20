from __future__ import annotations

import time
import tomllib
from pathlib import Path

from .internal.bundle import build_bundle_model, load_source_bundle
from .internal.constants import SUPPORTED_TARGETS
from .internal.files import (
    apply_plan_atomic,
    build_file_hashes,
    collect_plan_files,
    hash_bytes,
    read_existing_files,
)
from .internal.state import compute_input_hash, is_cache_hit, state_path, write_state
from .internal.sync_gate import validate_sync_output
from .internal.sync_report import print_bundle_diff, print_sync_header, print_sync_report


def _hash_file(path: Path) -> str:
    return hash_bytes(path.read_bytes())


def sync_target(
    *,
    target: str,
    source_root: Path,
    output_root: Path,
    apply: bool,
    check: bool,
    show_diff: bool,
    allow_overwrite_source: bool,
) -> bool:
    started = time.monotonic()
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
    file_hashes = build_file_hashes(planned_files)
    input_hash = compute_input_hash(target, source_root, file_hashes)

    cache_hit = False
    if apply and resolved_output.exists():
        cache_hit = is_cache_hit(
            output_root=resolved_output,
            expected_target=target,
            expected_source_root=resolved_source,
            expected_input_hash=input_hash,
            expected_file_hashes=file_hashes,
            hash_file_fn=_hash_file,
        )

    if cache_hit:
        duration_ms = int((time.monotonic() - started) * 1000)
        print_sync_header(
            target=target,
            source_root=source_root,
            output_root=output_root,
            planned_files=len(planned_files),
            added=0,
            changed=0,
            removed=0,
        )
        print("--- apply: cache hit, synchronized platform config is up to date.")
        print_sync_report(
            target=target,
            source_root=source_root,
            output_root=output_root,
            planned_files=len(planned_files),
            added=0,
            changed=0,
            removed=0,
            cache_hit=True,
            applied=False,
            duration_ms=duration_ms,
        )
        return False

    existing_files = read_existing_files(output_root)
    sync_state_rel = state_path(output_root).relative_to(output_root).as_posix()
    existing_files.pop(sync_state_rel, None)

    planned_keys = set(planned_files)
    existing_keys = set(existing_files)
    added = sorted(planned_keys - existing_keys)
    removed = sorted(existing_keys - planned_keys)
    changed = sorted(
        rel for rel in (planned_keys & existing_keys) if planned_files[rel] != existing_files[rel]
    )

    print_sync_header(
        target=target,
        source_root=source_root,
        output_root=output_root,
        planned_files=len(planned_files),
        added=len(added),
        changed=len(changed),
        removed=len(removed),
    )

    if show_diff:
        generated_bundle = planned_files["meta/bundle.toml"].decode("utf-8")
        print_bundle_diff(output_root, generated_bundle)

    if check:
        duration_ms = int((time.monotonic() - started) * 1000)
        if added or changed or removed:
            print("--- check: drift detected, synchronized platform config is stale.")
        else:
            print("--- check: synchronized platform config is up to date.")
        print_sync_report(
            target=target,
            source_root=source_root,
            output_root=output_root,
            planned_files=len(planned_files),
            added=len(added),
            changed=len(changed),
            removed=len(removed),
            cache_hit=False,
            applied=False,
            duration_ms=duration_ms,
        )
        return bool(added or changed or removed)

    if not apply:
        print("--- dry-run: no files were written.")
        duration_ms = int((time.monotonic() - started) * 1000)
        print_sync_report(
            target=target,
            source_root=source_root,
            output_root=output_root,
            planned_files=len(planned_files),
            added=len(added),
            changed=len(changed),
            removed=len(removed),
            cache_hit=False,
            applied=False,
            duration_ms=duration_ms,
        )
        return False

    apply_plan_atomic(output_root, planned_files)
    validate_sync_output(
        output_root=output_root,
        planned_files=planned_files,
        model=model,
        target=target,
    )
    write_state(
        output_root=output_root,
        target=target,
        source_root=source_root,
        input_hash=input_hash,
        file_hashes=file_hashes,
        planned_file_count=len(planned_files),
    )
    print("--- apply: synchronized platform config.")
    duration_ms = int((time.monotonic() - started) * 1000)
    print_sync_report(
        target=target,
        source_root=source_root,
        output_root=output_root,
        planned_files=len(planned_files),
        added=len(added),
        changed=len(changed),
        removed=len(removed),
        cache_hit=False,
        applied=True,
        duration_ms=duration_ms,
    )
    return False


def run_generation(
    *,
    target: str,
    source_root: Path,
    windows_output_root: Path,
    android_output_root: Path,
    apply: bool,
    check: bool,
    show_diff: bool,
    allow_overwrite_source: bool,
) -> int:
    if not source_root.exists():
        print(f"Source root does not exist: {source_root}")
        return 1

    targets: list[str] = list(SUPPORTED_TARGETS) if target == "both" else [target]
    drift_detected = False
    try:
        for item in targets:
            output_root = (
                windows_output_root.resolve()
                if item == "windows"
                else android_output_root.resolve()
            )
            drift_detected = (
                sync_target(
                    target=item,
                    source_root=source_root.resolve(),
                    output_root=output_root,
                    apply=apply,
                    check=check,
                    show_diff=show_diff,
                    allow_overwrite_source=allow_overwrite_source,
                )
                or drift_detected
            )
    except (FileNotFoundError, ValueError, tomllib.TOMLDecodeError) as error:
        print(str(error))
        return 1
    if check and drift_detected:
        return 1
    return 0
