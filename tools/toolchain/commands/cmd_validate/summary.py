from __future__ import annotations

import json
from dataclasses import asdict, dataclass, field
from datetime import UTC, datetime
from pathlib import Path
from time import monotonic

from .plan import ValidationPlan


def utc_now_iso() -> str:
    return datetime.now(UTC).isoformat()


def to_repo_relative(repo_root: Path, path: Path) -> str:
    try:
        return str(path.resolve().relative_to(repo_root.resolve())).replace("\\", "/")
    except ValueError:
        return str(path.resolve())


@dataclass
class StepResult:
    name: str
    command: str
    status: str
    exit_code: int | None
    duration_ms: int
    log_path: str
    key_error_lines: list[str] = field(default_factory=list)


@dataclass
class TrackResult:
    name: str
    kind: str
    app: str
    profile: str | None
    build_dir: str | None
    cmake_args: list[str]
    status: str
    exit_code: int | None
    duration_ms: int
    steps: list[StepResult] = field(default_factory=list)
    artifacts: dict[str, str] = field(default_factory=dict)


class ValidationSummaryWriter:
    def __init__(self, repo_root: Path, layout, plan: ValidationPlan, scope_paths: list[str]):
        self.repo_root = repo_root
        self.layout = layout
        self.plan = plan
        self.scope_paths = list(scope_paths)
        self._lines: list[str] = [
            f"run_name: {plan.run_name}",
            f"plan_path: {to_repo_relative(repo_root, plan.plan_path)}",
            f"scope_paths: {', '.join(scope_paths)}",
            "",
        ]
        self.flush_output_log()

    def flush_output_log(self) -> None:
        self.layout.logs_dir.mkdir(parents=True, exist_ok=True)
        self.layout.output_log_path.write_text(
            "\n".join(self._lines).rstrip() + "\n",
            encoding="utf-8",
        )

    def add_line(self, text: str = "") -> None:
        self._lines.append(text)
        self.flush_output_log()

    def add_track_start(self, *, index: int, total: int, track) -> None:
        self.add_line(f"[track {index}/{total}] {track.name} kind={track.kind} app={track.app}")

    def add_step(self, *, track_name: str, step: StepResult) -> None:
        prefix = "[ok]" if step.status == "completed" else "[failed]"
        self.add_line(
            f"{prefix} {track_name}/{step.name} exit={step.exit_code} "
            f"duration_ms={step.duration_ms} log={step.log_path}"
        )
        for line in step.key_error_lines:
            self.add_line(f"  key_error: {line}")

    def add_track_end(self, *, track: TrackResult) -> None:
        self.add_line(
            f"[track done] {track.name} status={track.status} "
            f"exit={track.exit_code} duration_ms={track.duration_ms}"
        )
        self.add_line("")

    def add_run_end(self, *, success: bool, exit_code: int) -> None:
        self.add_line(f"success: {str(success).lower()}")
        self.add_line(f"exit_code: {exit_code}")
        self.add_line(f"summary_json: {to_repo_relative(self.repo_root, self.layout.summary_json_path)}")

    def append_full_log(self, *, track_name: str, step_name: str, step_log_path: Path) -> None:
        try:
            content = step_log_path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            content = ""
        with self.layout.output_full_log_path.open("a", encoding="utf-8") as handle:
            handle.write(f"===== {track_name}/{step_name} =====\n")
            if content:
                handle.write(content.rstrip() + "\n")
            handle.write("\n")

    def write_summary(self, payload: dict) -> None:
        self.layout.summary_json_path.write_text(
            json.dumps(payload, indent=2, ensure_ascii=False),
            encoding="utf-8",
        )


def build_failures(track_results: list[TrackResult]) -> list[dict]:
    failures: list[dict] = []
    for track in track_results:
        for step in track.steps:
            if step.status == "completed":
                continue
            failures.append(
                {
                    "track": track.name,
                    "step": step.name,
                    "exit_code": step.exit_code,
                    "command": step.command,
                    "log_path": step.log_path,
                    "key_error_lines": list(step.key_error_lines),
                }
            )
            break
    return failures


def write_summary_payload(
    *,
    writer: ValidationSummaryWriter,
    repo_root: Path,
    command_text: str,
    plan: ValidationPlan,
    scope_paths: list[str],
    track_results: list[TrackResult],
    started_at_iso: str,
    started_at: float,
    exit_code: int,
    success: bool | None = None,
) -> None:
    final_success = success if success is not None else all(
        track.status == "completed" for track in track_results
    )
    payload = {
        "schema_version": 1,
        "command": command_text,
        "plan_path": to_repo_relative(repo_root, plan.plan_path),
        "scope_paths": list(scope_paths),
        "success": final_success,
        "exit_code": 0 if final_success else exit_code,
        "started_at": started_at_iso,
        "finished_at": utc_now_iso(),
        "duration_ms": int((monotonic() - started_at) * 1000),
        "tracks": [asdict(track) for track in track_results],
        "failures": build_failures(track_results),
    }
    writer.write_summary(payload)
