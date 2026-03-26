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

    process = subprocess.Popen(
        cmd,
        cwd=ctx.repo_root,
        env=ctx.setup_env(),
        stdout=None,
        stderr=None,
    )
    stage_start = time.monotonic()
    next_heartbeat = stage_start + 30.0

    while True:
        return_code = process.poll()
        if return_code is not None:
            return int(return_code)

        now = time.monotonic()
        if timeout_seconds is not None and timeout_seconds > 0:
            remaining_seconds = timeout_seconds - (now - start_time)
            if remaining_seconds <= 0:
                _terminate_process(process)
                print("--- tidy-batch: refresh stage timeout reached.")
                return 124

        if now >= next_heartbeat:
            elapsed = int(now - stage_start)
            print(
                f"--- tidy-batch: refresh still running for {batch_id} "
                f"({elapsed}s elapsed)."
            )
            next_heartbeat = now + 30.0
        time.sleep(1.0)


def _terminate_process(process: subprocess.Popen) -> None:
    try:
        process.terminate()
        process.wait(timeout=5)
        return
    except Exception:
        pass
    try:
        process.kill()
        process.wait(timeout=5)
    except Exception:
        pass
