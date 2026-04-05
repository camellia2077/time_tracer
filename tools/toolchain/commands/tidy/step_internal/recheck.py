from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from ....core.context import Context
from ....core.executor import run_command
from ....services import log_parser
from .. import clang_tidy_config
from ..refresh_internal import refresh_runner as tidy_refresh_runner
from ...shared import tidy as tidy_shared
from ..tasking.task_log import task_artifact_paths
from ..tasking.task_model import (
    build_task_draft_from_diagnostics,
    finalize_task_record,
    render_text,
    render_toon,
    task_record_to_dict,
)


@dataclass(frozen=True, slots=True)
class TaskRecheckResult:
    ok: bool
    exit_code: int
    log_path: Path
    remaining_diagnostics: tuple[dict, ...]
    diagnostics: tuple[dict, ...]


def run_task_recheck(
    ctx: Context,
    *,
    app_name: str,
    parsed,
    tidy_build_dir_name: str,
    source_scope: str | None,
    config_file: str | None = None,
    strict_config: bool = False,
    refresh_runner=tidy_refresh_runner,
    run_command_fn=run_command,
    extract_diagnostics_fn=log_parser.extract_diagnostics,
) -> TaskRecheckResult:
    tidy_layout = ctx.get_tidy_layout(app_name, tidy_build_dir_name)
    build_dir = tidy_layout.root
    ensure_ret = refresh_runner.ensure_analysis_compile_db(
        ctx=ctx,
        app_name=app_name,
        build_dir=build_dir,
        build_dir_name=tidy_build_dir_name,
        source_scope=source_scope,
        config_file=config_file,
        strict_config=strict_config,
    )
    log_path = tidy_layout.automation_dir / f"{parsed.batch_id}_task_{parsed.task_id}_recheck.log"
    if ensure_ret != 0:
        return TaskRecheckResult(
            ok=False,
            exit_code=ensure_ret,
            log_path=log_path,
            remaining_diagnostics=(),
            diagnostics=(),
        )

    source_files = collect_recheck_files(parsed)
    if not source_files:
        return TaskRecheckResult(
            ok=False,
            exit_code=1,
            log_path=log_path,
            remaining_diagnostics=(),
            diagnostics=(),
        )

    compile_db_dir = refresh_runner.analysis_compile_db.ensure_analysis_compile_db(build_dir)
    checks = [check for check in parsed.checks if check]
    header_filter = (ctx.config.tidy.header_filter_regex or "").strip()
    if not header_filter:
        header_filter = r"^(?!.*[\\/]_deps[\\/]).*"

    cmd = [
        "clang-tidy",
        "-p",
        str(compile_db_dir),
        f"-header-filter={header_filter}",
    ]
    cmd.extend(
        clang_tidy_config.build_config_file_args(
            ctx,
            config_file=config_file,
            strict_config=strict_config,
        )
    )
    if checks and all(str(check).startswith("clang-diagnostic-") for check in checks):
        cmd.append("--allow-no-checks")
    if checks:
        cmd.append(f"-checks=-*,{','.join(checks)}")
    cmd.append("--quiet")
    cmd.extend(str(path) for path in source_files)

    recheck_ret = run_command_fn(
        cmd,
        cwd=ctx.repo_root,
        env=ctx.setup_env(),
        log_file=log_path,
        output_mode="quiet",
    )
    try:
        log_lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines()
    except OSError:
        log_lines = []
    diagnostics = extract_diagnostics_fn(log_lines)
    remaining = tuple(match_remaining_diagnostics(parsed, diagnostics))
    return TaskRecheckResult(
        ok=(recheck_ret == 0 and not remaining),
        exit_code=recheck_ret,
        log_path=log_path,
        remaining_diagnostics=remaining,
        diagnostics=tuple(diagnostics),
    )


def collect_recheck_files(parsed) -> list[Path]:
    files: list[Path] = []
    seen: set[str] = set()
    raw_paths = [parsed.source_file, *(diagnostic.file for diagnostic in parsed.diagnostics)]
    for raw_path in raw_paths:
        text = str(raw_path or "").strip()
        if not text:
            continue
        path = Path(text)
        normalized = path_key(path)
        if normalized in seen:
            continue
        seen.add(normalized)
        files.append(path)
    return files


def match_remaining_diagnostics(parsed, diagnostics: list[dict]) -> list[dict]:
    expected_keys = {
        (
            path_key(diagnostic.file or parsed.source_file),
            int(diagnostic.line),
            int(diagnostic.col),
            diagnostic.check,
        )
        for diagnostic in parsed.diagnostics
    }
    remaining: list[dict] = []
    for diagnostic in diagnostics:
        current_key = (
            path_key(diagnostic.get("file", "") or parsed.source_file),
            int(diagnostic.get("line", 0)),
            int(diagnostic.get("col", 0)),
            str(diagnostic.get("check", "")).strip(),
        )
        if current_key in expected_keys:
            remaining.append(diagnostic)
    return remaining


def path_key(path_like) -> str:
    return str(Path(str(path_like))).replace("\\", "/").lower()


def rewrite_task_artifacts_from_recheck(
    *,
    task_ctx,
    diagnostics: list[dict],
    resolved_task_path: Path,
) -> None:
    draft = build_task_draft_from_diagnostics(diagnostics)
    if draft is None:
        return
    refreshed_record = finalize_task_record(
        draft,
        task_id=task_ctx.parsed_task.task_id,
        batch_id=task_ctx.parsed_task.batch_id,
        queue_generation=task_ctx.current_queue_generation,
        workspace=task_ctx.tidy_build_dir_name,
        source_scope=task_ctx.source_scope,
    )
    json_path = resolved_task_path.with_suffix(".json")
    tidy_shared.write_json_dict(json_path, task_record_to_dict(refreshed_record))
    existing_artifacts = {artifact.suffix.lower() for artifact in task_artifact_paths(json_path)}
    base_path = json_path.with_suffix("")
    if ".log" in existing_artifacts:
        base_path.with_suffix(".log").write_text(
            render_text(refreshed_record),
            encoding="utf-8",
        )
    if ".toon" in existing_artifacts:
        base_path.with_suffix(".toon").write_text(
            render_toon(refreshed_record),
            encoding="utf-8",
        )
