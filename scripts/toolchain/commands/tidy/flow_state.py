from datetime import UTC, datetime
from pathlib import Path

from ..shared import tidy as tidy_shared


def new_state(
    app_name: str,
    process_all: bool,
    n: int,
    resume: bool,
    test_every: int,
    concise: bool,
    jobs: int | None,
    parse_workers: int | None,
    keep_going: bool,
    run_tidy_fix: bool,
    tidy_fix_limit: int,
    verify_build_dir: str,
    profile_name: str | None,
    kill_build_procs: bool,
    state_path: Path,
) -> dict:
    mode = "all" if process_all else f"n:{n}"
    now = utc_now_iso()
    state = {
        "app": app_name,
        "mode": mode,
        "resume": resume,
        "test_every": test_every,
        "concise": concise,
        "jobs": jobs,
        "parse_workers": parse_workers,
        "keep_going": keep_going,
        "run_tidy_fix": run_tidy_fix,
        "tidy_fix_limit": tidy_fix_limit,
        "verify_build_dir": verify_build_dir,
        "profile": profile_name,
        "tidy_fix_exit_code": None,
        "kill_build_procs": kill_build_procs,
        "state_file": str(state_path),
        "status": "running",
        "phase": "init",
        "started_at": now,
        "updated_at": now,
        "exit_code": None,
        "blocked_task_id": None,
        "pending_task_ids": [],
        "rename_candidates": 0,
        "cleaned_empty_task_ids": [],
        "steps": {
            "prepare_tasks": {"status": "pending", "exit_code": None},
            "rename": {"status": "pending", "exit_code": None},
            "verify": {"status": "pending", "exit_code": None},
            "loop": {"status": "pending", "exit_code": None},
            "clean": {"status": "pending", "exit_code": None},
        },
    }
    write_state(state_path, state)
    return state


def set_phase(state: dict, phase_name: str) -> None:
    state["phase"] = phase_name
    state["updated_at"] = utc_now_iso()


def set_step(
    state: dict,
    step_name: str,
    status: str,
    exit_code: int | None = None,
) -> None:
    state["steps"][step_name]["status"] = status
    state["steps"][step_name]["exit_code"] = exit_code
    state["updated_at"] = utc_now_iso()


def finish(
    state: dict,
    state_path: Path,
    exit_code: int,
    pending_task_ids: list[str],
    blocked_task_id: str | None = None,
) -> int:
    state["status"] = "completed" if exit_code == 0 else "stopped"
    state["exit_code"] = exit_code
    state["blocked_task_id"] = blocked_task_id
    state["pending_task_ids"] = pending_task_ids
    state["phase"] = "done"
    state["updated_at"] = utc_now_iso()
    write_state(state_path, state)

    print(
        f"--- tidy-flow summary: exit={exit_code}, pending={len(pending_task_ids)}, "
        f"blocked={blocked_task_id if blocked_task_id else '-'}"
    )
    print(f"--- tidy-flow state: {state_path}")
    return exit_code


def write_state(state_path: Path, state: dict) -> None:
    tidy_shared.write_json_dict(state_path, state)


def utc_now_iso() -> str:
    return datetime.now(UTC).isoformat()
