from datetime import UTC, datetime
from pathlib import Path

from ..shared import tidy as tidy_shared


def load_state(state_path: Path) -> dict:
    default_state = {
        "version": 1,
        "batches_since_full": 0,
        "processed_batches": [],
        "last_batch": None,
        "last_full_at": None,
        "last_full_batch": None,
        "last_full_reason": "",
        "full_every": None,
        "last_seen_build_log_mtime_ns": None,
        "last_seen_rename_report_mtime_ns": None,
        "updated_at": None,
    }
    payload = tidy_shared.read_json_dict(state_path)
    if payload is None:
        return default_state
    merged = dict(default_state)
    merged.update(payload)
    if not isinstance(merged.get("processed_batches"), list):
        merged["processed_batches"] = []
    return merged


def ensure_processed_batches(state: dict) -> list[str]:
    processed = state.get("processed_batches")
    if not isinstance(processed, list):
        processed = []
        state["processed_batches"] = processed
    return processed


def register_batch(
    state: dict,
    batch_name: str | None,
    max_batches: int = 300,
) -> bool:
    if not batch_name:
        return False

    processed = ensure_processed_batches(state)
    if batch_name in processed:
        return True

    processed.append(batch_name)
    state["processed_batches"] = processed[-max_batches:]
    state["batches_since_full"] = int(state.get("batches_since_full", 0)) + 1
    state["last_batch"] = batch_name
    return False


def cadence_due(
    state: dict,
    batch_name: str | None,
    already_processed: bool,
    full_every: int,
) -> bool:
    return (
        bool(batch_name)
        and not already_processed
        and full_every > 0
        and int(state.get("batches_since_full", 0)) >= full_every
    )


def resolve_full_reasons(
    *,
    force_full: bool,
    final_full: bool,
    cadence_is_due: bool,
    full_every: int,
) -> list[str]:
    reasons: list[str] = []
    if force_full:
        reasons.append("force_full")
    if final_full:
        reasons.append("final_full")
    if cadence_is_due:
        reasons.append(f"cadence_{full_every}")
    return reasons


def write_state(state_path: Path, state: dict) -> None:
    tidy_shared.write_json_dict(state_path, state)


def print_preview(
    batch_name: str | None,
    touched_files: list[Path],
    incremental_files: list[Path],
    neighbor_scope: str,
    full_every: int,
    cadence_due: bool,
    full_reasons: list[str],
    keep_going: bool,
) -> None:
    print("--- tidy-refresh dry-run ---")
    if batch_name:
        print(f"batch: {batch_name}")
        print(f"touched files: {len(touched_files)}")
        print(
            f"incremental compile units: {len(incremental_files)} (neighbor_scope={neighbor_scope})"
        )
        preview_files = [str(path) for path in incremental_files[:10]]
        if preview_files:
            print("sample files:")
            for item in preview_files:
                print(f"  - {item}")
        if len(incremental_files) > 10:
            print(f"  - ... ({len(incremental_files) - 10} more)")
    else:
        print("batch: <none>")
    print(
        f"full cadence: every {full_every} batch(es), cadence_due={cadence_due}, "
        f"force/final reasons={','.join(full_reasons) if full_reasons else '-'}"
    )
    print(f"incremental keep_going: {keep_going}")


def utc_now_iso() -> str:
    return datetime.now(UTC).isoformat()
