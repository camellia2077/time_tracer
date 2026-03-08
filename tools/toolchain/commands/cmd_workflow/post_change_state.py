from __future__ import annotations

import json
from datetime import UTC, datetime
from pathlib import Path

from ...core.context import Context
from ...services.change_policy import ChangePolicyDecision
from ...services.suite_registry import needs_suite_build, resolve_suite_runner_name
from ..shared.result_reporting import print_failure_report, print_result_paths


def resolve_state_path(ctx: Context, app_name: str, build_dir_name: str) -> Path:
    return ctx.get_app_dir(app_name) / build_dir_name / "post_change_last.json"


def utc_now_iso() -> str:
    return datetime.now(UTC).isoformat()


def write_state(state_path: Path, state: dict) -> None:
    state_path.parent.mkdir(parents=True, exist_ok=True)
    state_path.write_text(
        json.dumps(state, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )


def set_step(state: dict, step_name: str, status: str, exit_code: int | None = None) -> None:
    state["steps"][step_name]["status"] = status
    state["steps"][step_name]["exit_code"] = exit_code
    state["updated_at"] = utc_now_iso()


def set_step_duration_ms(state: dict, step_name: str, duration_ms: int) -> None:
    timings = state.setdefault("metrics", {}).setdefault("timings_ms", {})
    timings[step_name] = int(duration_ms)
    state["updated_at"] = utc_now_iso()


def set_total_duration_ms(state: dict, duration_ms: int) -> None:
    timings = state.setdefault("metrics", {}).setdefault("timings_ms", {})
    timings["total"] = int(duration_ms)
    state["updated_at"] = utc_now_iso()


def set_artifact_sizes(state: dict, artifact_sizes: dict) -> None:
    metrics = state.setdefault("metrics", {})
    metrics["artifact_sizes"] = artifact_sizes
    state["updated_at"] = utc_now_iso()


def configure_command_text(decision: ChangePolicyDecision, profile_name: str | None) -> str:
    cmd = [
        "python",
        "tools/run.py",
        "configure",
        "--app",
        decision.app_name,
        "--build-dir",
        decision.build_dir_name,
    ]
    if profile_name:
        cmd.extend(["--profile", profile_name])
    for arg in decision.cmake_args:
        cmd.append(f"--cmake-args={arg}")
    return " ".join(cmd)


def build_command_text(decision: ChangePolicyDecision, profile_name: str | None) -> str:
    cmd = [
        "python",
        "tools/run.py",
        "build",
        "--app",
        decision.app_name,
        "--build-dir",
        decision.build_dir_name,
    ]
    if profile_name:
        cmd.extend(["--profile", profile_name])
    if decision.cmake_args and not decision.need_configure:
        for arg in decision.cmake_args:
            cmd.append(f"--cmake-args={arg}")
    return " ".join(cmd)


def test_command_text(decision: ChangePolicyDecision) -> str:
    suite_runner_name = resolve_suite_runner_name(decision.app_name) or decision.app_name
    cmd = [
        "python",
        "test/run.py",
        "suite",
        "--suite",
        suite_runner_name,
        "--agent",
        "--build-dir",
        decision.build_dir_name,
    ]
    if needs_suite_build(decision.app_name):
        cmd.extend(["--with-build", "--skip-configure"])
    return " ".join(cmd)


def new_state(
    decision: ChangePolicyDecision,
    state_path: Path,
    profile_name: str | None,
    run_tests: str,
    script_changes: str,
    concise: bool,
    dry_run: bool,
    kill_build_procs: bool,
) -> dict:
    now = utc_now_iso()
    state = {
        "app": decision.app_name,
        "build_dir": decision.build_dir_name,
        "state_file": str(state_path),
        "status": "running",
        "started_at": now,
        "updated_at": now,
        "exit_code": None,
        "options": {
            "profile": profile_name,
            "run_tests": run_tests,
            "script_changes": script_changes,
            "concise": concise,
            "dry_run": dry_run,
            "kill_build_procs": kill_build_procs,
            "cmake_args": decision.cmake_args,
        },
        "decision": {
            "changed_files": decision.changed_files,
            "reasons": decision.reasons,
            "need_configure": decision.need_configure,
            "need_build": decision.need_build,
            "need_test": decision.need_test,
            "cmake_args": decision.cmake_args,
        },
        "failed_stage": None,
        "failed_command": None,
        "error_summary": None,
        "next_action": None,
        "metrics": {
            "timings_ms": {
                "configure": None,
                "build": None,
                "test": None,
                "total": None,
            },
            "artifact_sizes": {
                "candidate_dirs": [],
                "artifacts": [],
                "count": 0,
                "total_bytes": 0,
            },
        },
        "steps": {
            "configure": {
                "status": "pending" if decision.need_configure else "skipped",
                "exit_code": None,
                "command": configure_command_text(decision, profile_name),
            },
            "build": {
                "status": "pending" if decision.need_build else "skipped",
                "exit_code": None,
                "command": build_command_text(decision, profile_name),
            },
            "test": {
                "status": "pending" if decision.need_test else "skipped",
                "exit_code": None,
                "command": test_command_text(decision),
            },
        },
    }
    write_state(state_path=state_path, state=state)
    return state


def finish(
    state: dict,
    state_path: Path,
    exit_code: int,
    status: str,
    repo_root: Path | None = None,
) -> int:
    state["status"] = status
    state["exit_code"] = exit_code
    state["updated_at"] = utc_now_iso()
    write_state(state_path=state_path, state=state)
    print(f"--- post-change state: {state_path}")
    if repo_root is not None:
        print_result_paths(
            app_name=str(state.get("app", "")),
            repo_root=repo_root,
            state_json=state_path,
        )
    return exit_code


def finish_failure(
    state: dict,
    state_path: Path,
    stage: str,
    exit_code: int,
    repo_root: Path,
) -> int:
    set_step(state, stage, "failed", exit_code)
    state["failed_stage"] = stage
    failed_command = str(state["steps"][stage]["command"])
    state["failed_command"] = failed_command
    state["error_summary"] = (
        f"{stage} failed with exit code {exit_code}. Check command output above for key errors."
    )
    state["next_action"] = "Fix errors and rerun: python tools/run.py post-change"
    print_failure_report(
        command=failed_command,
        exit_code=exit_code,
        next_action=state["next_action"],
        app_name=str(state.get("app", "")),
        repo_root=repo_root,
        state_json=state_path,
        fallback_key_error_hint="See command output above for key errors.",
    )
    return finish(
        state=state,
        state_path=state_path,
        exit_code=exit_code,
        status="stopped",
        repo_root=repo_root,
    )
