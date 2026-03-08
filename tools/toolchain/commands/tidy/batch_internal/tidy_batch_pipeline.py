from __future__ import annotations

import subprocess
import sys
import time


def should_run_stage(next_stage: str, stage: str, stage_order: tuple[str, ...]) -> bool:
    try:
        return stage_order.index(stage) >= stage_order.index(next_stage)
    except ValueError:
        return True


def timeout_reached(start_time: float, timeout_seconds: int | None) -> bool:
    if timeout_seconds is None or timeout_seconds <= 0:
        return False
    return (time.monotonic() - start_time) >= timeout_seconds


def run_refresh_stage(
    *,
    ctx,
    refresh_command_cls,
    app_name: str,
    batch_id: str,
    full_every: int,
    keep_going: bool | None,
    source_scope: str | None,
    tidy_build_dir_name: str,
    start_time: float,
    timeout_seconds: int | None,
) -> int:
    if timeout_seconds is None or timeout_seconds <= 0:
        return refresh_command_cls(ctx).execute(
            app_name=app_name,
            batch_id=batch_id,
            full_every=full_every,
            keep_going=keep_going,
            source_scope=source_scope,
            build_dir_name=tidy_build_dir_name,
        )

    remaining_seconds = timeout_seconds - (time.monotonic() - start_time)
    if remaining_seconds <= 0:
        return 124

    cmd = [
        sys.executable,
        str(ctx.repo_root / "tools" / "run.py"),
        "tidy-refresh",
        "--app",
        app_name,
        "--batch-id",
        batch_id,
        "--full-every",
        str(full_every),
        "--build-dir",
        tidy_build_dir_name,
    ]
    if source_scope:
        cmd += ["--source-scope", source_scope]
    if keep_going is True:
        cmd.append("--keep-going")
    elif keep_going is False:
        cmd.append("--no-keep-going")

    try:
        completed = subprocess.run(
            cmd,
            cwd=ctx.repo_root,
            env=ctx.setup_env(),
            check=False,
            timeout=max(1, int(remaining_seconds)),
        )
        return int(completed.returncode)
    except subprocess.TimeoutExpired:
        print("--- tidy-batch: refresh stage timeout reached.")
        return 124
