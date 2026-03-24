from __future__ import annotations

import sys
import traceback
from dataclasses import dataclass, field
from pathlib import Path
from time import monotonic

from ...core.executor import run_command
from ...services.log_analysis import extract_key_error_lines_from_log
from .summary import StepResult, ValidationSummaryWriter, to_repo_relative


def shell_join(cmd: list[str]) -> str:
    return " ".join(str(part) for part in cmd)


def safe_segment(text: str) -> str:
    safe = "".join(ch if ch.isalnum() or ch in "._-" else "_" for ch in (text or ""))
    safe = safe.replace(".", "_").strip("._-")
    return safe or "item"


@dataclass
class StepCapture:
    command_texts: list[str] = field(default_factory=list)

    def run_command(self, cmd: list[str], **kwargs) -> int:
        self.command_texts.append(shell_join(cmd))
        return run_command(cmd, **kwargs)


class TeeStream:
    def __init__(self, primary, mirror):
        self._primary = primary
        self._mirror = mirror

    @property
    def encoding(self):
        encoding = getattr(self._primary, "encoding", None)
        if encoding:
            return encoding
        return getattr(self._mirror, "encoding", None)

    def write(self, text: str) -> int:
        written = self._primary.write(text)
        self._mirror.write(text)
        return written

    def flush(self) -> None:
        self._primary.flush()
        self._mirror.flush()


def run_logged_step(
    *,
    repo_root: Path,
    track_name: str,
    track_index: int,
    step_name: str,
    writer: ValidationSummaryWriter,
    verbose: bool,
    action,
    fallback_command: str = "",
) -> StepResult:
    step_started_at = monotonic()
    track_segment = f"{track_index:02d}_{safe_segment(track_name)}"
    step_segment = safe_segment(step_name)
    step_log_path = writer.layout.tracks_dir / track_segment / f"{step_segment}.log"
    step_log_path.parent.mkdir(parents=True, exist_ok=True)

    capture = StepCapture()
    exit_code = 0
    with step_log_path.open("w", encoding="utf-8") as log_handle:
        stdout_stream = log_handle if not verbose else TeeStream(log_handle, sys.stdout)
        stderr_stream = log_handle if not verbose else TeeStream(log_handle, sys.stderr)
        saved_stdout = sys.stdout
        saved_stderr = sys.stderr
        sys.stdout = stdout_stream
        sys.stderr = stderr_stream
        try:
            try:
                exit_code = int(action(capture.run_command))
            except Exception:
                traceback.print_exc()
                exit_code = 1
        finally:
            sys.stdout = saved_stdout
            sys.stderr = saved_stderr

    command_text = " && ".join(capture.command_texts) if capture.command_texts else fallback_command
    status = "completed" if exit_code == 0 else "failed"
    key_error_lines = extract_key_error_lines_from_log(step_log_path) if exit_code != 0 else []
    step_result = StepResult(
        name=step_name,
        command=command_text,
        status=status,
        exit_code=exit_code,
        duration_ms=int((monotonic() - step_started_at) * 1000),
        log_path=to_repo_relative(repo_root, step_log_path),
        key_error_lines=key_error_lines,
    )
    writer.add_step(track_name=track_name, step=step_result)
    writer.append_full_log(track_name=track_name, step_name=step_name, step_log_path=step_log_path)
    return step_result
