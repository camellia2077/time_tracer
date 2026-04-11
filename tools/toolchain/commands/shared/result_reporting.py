from __future__ import annotations

import os
import sys
from pathlib import Path

from ...core.generated_paths import resolve_test_result_layout_for_app
from ...services.log_analysis import summarize_failure_log


def _write_stdout_safe(text: str) -> None:
    candidates = [sys.stdout]
    fallback = getattr(sys, "__stdout__", None)
    if fallback is not None and fallback is not sys.stdout:
        candidates.append(fallback)

    for stream in candidates:
        if stream is None:
            continue
        try:
            stream.write(text)
            return
        except UnicodeEncodeError:
            encoding = getattr(stream, "encoding", None) or "utf-8"
            safe_text = text.encode(encoding, errors="replace").decode(
                encoding,
                errors="replace",
            )
            try:
                stream.write(safe_text)
                return
            except (BrokenPipeError, OSError, ValueError):
                continue
        except (BrokenPipeError, OSError, ValueError):
            continue


def _emit_line(text: str) -> None:
    _write_stdout_safe(f"{text}\n")


def resolve_result_paths(repo_root: Path, app_name: str) -> dict[str, Path]:
    layout = resolve_test_result_layout_for_app(repo_root, app_name)
    return {
        "output_root": layout.root,
        "result_json": layout.result_json_path,
        "aggregated_log": layout.output_log_path,
        "output_log": layout.output_log_path,
    }


def print_result_paths(
    *,
    app_name: str,
    repo_root: Path,
    state_json: Path | None = None,
) -> None:
    paths = resolve_result_paths(repo_root=repo_root, app_name=app_name)
    _emit_line("--- result visibility")
    if state_json is not None:
        _emit_line(f"state_json: {state_json}")
    _emit_line(f"result_json (summary): {paths['result_json']}")
    _emit_line(f"aggregated_log: {paths['aggregated_log']}")


def _append_github_step_summary(lines: list[str]) -> None:
    summary_path = str(os.environ.get("GITHUB_STEP_SUMMARY", "") or "").strip()
    if not summary_path:
        return
    try:
        with Path(summary_path).open("a", encoding="utf-8") as handle:
            handle.write("\n".join(lines) + "\n")
    except OSError:
        return


def print_verify_phase_summary(verify_phases: list[dict[str, object]]) -> None:
    if not verify_phases:
        return

    _emit_line("--- verify phase summary")
    markdown_lines = [
        "### Verify Phase Summary",
        "",
        "| Phase | Status | Exit | Category |",
        "| --- | --- | ---: | --- |",
    ]
    for phase in verify_phases:
        name = str(phase.get("name", "") or "")
        status = str(phase.get("status", "") or "")
        exit_code = int(phase.get("exit_code", 0) or 0)
        category = str(phase.get("category", "") or "")
        _emit_line(
            f"- {name}: status={status}, exit_code={exit_code}, category={category}"
        )
        markdown_lines.append(f"| `{name}` | `{status}` | `{exit_code}` | `{category}` |")
    _append_github_step_summary(markdown_lines)


def print_failure_report(
    *,
    command: str,
    exit_code: int,
    next_action: str,
    app_name: str,
    repo_root: Path,
    stage: str | None = None,
    state_json: Path | None = None,
    build_log_path: Path | None = None,
    fallback_key_error_hint: str = "See command output above.",
    include_result_json: bool = True,
) -> None:
    paths = resolve_result_paths(repo_root=repo_root, app_name=app_name)
    aggregated_log_path = paths["aggregated_log"]
    analysis_log_path = build_log_path or aggregated_log_path
    failure_summary = summarize_failure_log(analysis_log_path)

    _emit_line("=== FAILURE REPORT ===")
    if stage is not None:
        _emit_line(f"stage: {stage}")
    _emit_line(f"command: {command}")
    _emit_line(f"exit_code: {exit_code}")
    if state_json is not None:
        _emit_line(f"state_json: {state_json}")
    if include_result_json:
        _emit_line(f"result_json: {paths['result_json']}")
    if build_log_path is not None:
        _emit_line(f"build_log: {build_log_path}")
    _emit_line(f"aggregated_log: {aggregated_log_path}")
    if failure_summary.primary_error:
        _emit_line(f"primary_error: {failure_summary.primary_error}")
    if failure_summary.likely_fix:
        _emit_line(f"likely_fix: {failure_summary.likely_fix}")
    _emit_line("key_error_lines:")
    if failure_summary.key_error_lines:
        for line in failure_summary.key_error_lines:
            _emit_line(f"  - {line}")
    else:
        _emit_line(f"  - {fallback_key_error_hint}")
    _emit_line(f"next_action: {next_action}")
    _emit_line("======================")
