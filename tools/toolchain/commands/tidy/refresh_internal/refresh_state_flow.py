from __future__ import annotations

from pathlib import Path

from ....services import batch_state


def parse_optional_int(raw) -> int | None:
    try:
        return int(raw) if raw is not None else None
    except (TypeError, ValueError):
        return None


def write_refresh_state(
    *,
    command,
    next_state: dict,
    state_path: Path,
    build_dir: Path,
    full_every: int,
) -> None:
    next_state["full_every"] = full_every
    next_state["last_seen_build_log_mtime_ns"] = command._build_log_mtime_ns(build_dir)
    next_state["updated_at"] = command._utc_now_iso()
    command._write_state(state_path, next_state)
    print(f"--- tidy-refresh: state updated -> {state_path}")


def write_refresh_batch_state(
    *,
    command,
    app_name: str,
    batch_id: str | None,
    resolved_build_dir_name: str,
) -> None:
    batch_state_path = batch_state.update_state(
        ctx=command.ctx,
        app_name=app_name,
        tidy_build_dir_name=resolved_build_dir_name,
        batch_id=batch_id,
        last_refresh_ok=True,
        extra_fields={"last_refresh_build_dir": resolved_build_dir_name},
    )
    print(f"--- tidy-refresh: batch state updated -> {batch_state_path}")
