from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from ...core.executor import run_command
from ...services import batch_state, log_parser
from ...core.context import Context
from ..cmd_build import BuildCommand
from ..shared import tidy as tidy_shared
from .clean import CleanCommand
from .refresh_internal import refresh_runner as tidy_refresh_runner
from . import tidy_result as tidy_result_summary
from .task_auto_fix import run_task_auto_fix
from .task_log import list_task_paths, load_task_record, parse_task_log, resolve_task_log_path
from .workspace import resolve_workspace


@dataclass(frozen=True, slots=True)
class TaskRecheckResult:
    ok: bool
    exit_code: int
    log_path: Path
    remaining_diagnostics: tuple[dict, ...]


class TidyStepCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        *,
        app_name: str,
        task_log_path: str | None = None,
        batch_id: str | None = None,
        task_id: str | None = None,
        tidy_build_dir_name: str | None = None,
        source_scope: str | None = None,
        verify_build_dir_name: str | None = None,
        profile_name: str | None = None,
        concise: bool = False,
        kill_build_procs: bool = False,
        dry_run: bool = False,
        strict: bool = False,
    ) -> int:
        workspace = resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        tidy_layout = self.ctx.get_tidy_layout(app_name, workspace.build_dir_name)
        tasks_dir = tidy_layout.tasks_dir
        resolved_task_path = resolve_task_log_path(
            tasks_dir,
            task_log_path=task_log_path,
            batch_id=batch_id,
            task_id=task_id,
        )
        parsed = parse_task_log(resolved_task_path)
        batch_tasks_before = list_task_paths(tasks_dir, batch_id=parsed.batch_id)
        print(
            f"--- tidy-step: selected {parsed.batch_id}/task_{parsed.task_id} "
            f"({parsed.source_file or resolved_task_path})"
        )

        fix_result = run_task_auto_fix(
            self.ctx,
            app_name=app_name,
            task_log_path=str(resolved_task_path),
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            dry_run=dry_run,
            report_suffix="step",
        )
        print(
            f"--- tidy-task-fix: task={fix_result.task_id}, applied={fix_result.applied}, "
            f"previewed={fix_result.previewed}, skipped={fix_result.skipped}, failed={fix_result.failed}"
        )
        print(f"--- tidy-task-fix: report json -> {fix_result.json_path}")
        print(f"--- tidy-task-fix: report md   -> {fix_result.markdown_path}")
        if fix_result.failed > 0:
            if self._can_continue_after_fix_failures(fix_result):
                print(
                    "--- tidy-step: task auto-fix hit stale rename failures only; "
                    "continuing with verify/re-check for manual-close path."
                )
            else:
                print("--- tidy-step: task auto-fix failed.")
                return fix_result.exit_code(strict=strict)
        if fix_result.applied == 0 and fix_result.previewed == 0:
            print(
                "--- tidy-step: task auto-fix produced no supported edits; "
                "continuing with verify/re-check for manual-close path."
            )

        if dry_run:
            print("--- tidy-step: dry-run mode, skip verify/batch follow-up.")
            return 0

        print("--- tidy-step: running build sanity check...")
        verify_ret = BuildCommand(self.ctx).build(
            app_name=app_name,
            tidy=False,
            build_dir_name=verify_build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
        )
        if verify_ret != 0:
            print("--- tidy-step: build sanity check failed.")
            return verify_ret

        print("--- tidy-step: running post-build clang-tidy re-check...")
        recheck_result = self._run_task_recheck(
            app_name=app_name,
            parsed=parsed,
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
        )
        if not recheck_result.ok:
            print("--- tidy-step: task re-check failed or matching diagnostics remain.")
            print(f"--- tidy-step: re-check log -> {recheck_result.log_path}")
            for diagnostic in recheck_result.remaining_diagnostics[:5]:
                location = (
                    f"{diagnostic.get('file', parsed.source_file)}:"
                    f"{diagnostic.get('line', 0)}:{diagnostic.get('col', 0)}"
                )
                print(
                    "  - "
                    + f"{location} [{diagnostic.get('check', 'unknown')}] "
                    + str(diagnostic.get("message", "")).strip()
                )
            return recheck_result.exit_code or 1

        print("--- tidy-step: task re-check passed. Archiving task artifact(s)...")
        clean_ret = CleanCommand(self.ctx).execute(
            app_name=app_name,
            task_ids=[parsed.task_id],
            batch_id=parsed.batch_id,
            tidy_build_dir_name=workspace.build_dir_name,
        )
        if clean_ret != 0:
            print("--- tidy-step: clean/archive failed after successful re-check.")
            return clean_ret

        queue_head_after_close = self._build_queue_head(tasks_dir)
        self._sync_queue_snapshot_after_close(
            app_name=app_name,
            tidy_build_dir_name=workspace.build_dir_name,
            source_scope=workspace.source_scope,
            closed_batch_id=parsed.batch_id,
            closed_task_id=parsed.task_id,
            closed_task_path=resolved_task_path,
            recheck_log_path=recheck_result.log_path,
            queue_head_after_close=queue_head_after_close,
        )
        if len(batch_tasks_before) == 1:
            next_action = self._build_single_task_close_next_action(
                batch_id=parsed.batch_id,
                task_id=parsed.task_id,
                queue_head=queue_head_after_close,
            )
            print(f"--- tidy-step: {next_action}")
            step_state_path = tidy_layout.automation_dir / "tidy_step_last.json"
            tidy_shared.write_json_dict(
                step_state_path,
                {
                    "app": app_name,
                    "task_id": parsed.task_id,
                    "batch_id": parsed.batch_id,
                    "task_log": str(resolved_task_path),
                    "verify_exit_code": verify_ret,
                    "recheck_log": str(recheck_result.log_path),
                    "task_archived": True,
                    "single_task_batch_closed": True,
                    "historical_selection_stale_after_close": True,
                    "queue_requires_reresolve_after_close": True,
                    "next_queue_head_after_close": queue_head_after_close,
                    "next_action": next_action,
                },
            )
            print(f"--- tidy-step: step state -> {step_state_path}")
            return 0

        handoff_command = "python tools/run.py tidy-batch --app " f"{app_name}"
        if workspace.source_scope:
            handoff_command += f" --source-scope {workspace.source_scope}"
        if workspace.build_dir_name:
            handoff_command += f" --tidy-build-dir {workspace.build_dir_name}"
        handoff_command += f" --batch-id {parsed.batch_id} --preset sop"
        next_action = (
            f"Task {parsed.batch_id}/task_{parsed.task_id} is closed. If you continue with batch "
            f"handoff, run `{handoff_command}`. After refresh completes, the historical batch/task "
            "selection becomes stale; re-resolve the current smallest pending task from tasks/ "
            "before continuing, and do not reuse historical batch/task ids."
        )
        print(f"--- tidy-step: {next_action}")

        step_state_path = tidy_layout.automation_dir / "tidy_step_last.json"
        tidy_shared.write_json_dict(
            step_state_path,
            {
                "app": app_name,
                "task_id": parsed.task_id,
                "batch_id": parsed.batch_id,
                "task_log": str(resolved_task_path),
                "verify_exit_code": verify_ret,
                "recheck_log": str(recheck_result.log_path),
                "task_archived": True,
                "handoff_batch_id": parsed.batch_id,
                "queue_requires_reresolve_after_batch": True,
                "handoff_command": handoff_command,
                "next_action": next_action,
            },
        )
        print(f"--- tidy-step: step state -> {step_state_path}")
        return 0

    def _sync_queue_snapshot_after_close(
        self,
        *,
        app_name: str,
        tidy_build_dir_name: str,
        source_scope: str | None,
        closed_batch_id: str,
        closed_task_id: str,
        closed_task_path: Path,
        recheck_log_path: Path,
        queue_head_after_close: dict | None,
    ) -> None:
        transition_summary = self._build_close_transition_summary(
            batch_id=closed_batch_id,
            task_id=closed_task_id,
            queue_head=queue_head_after_close,
        )
        result_path = tidy_result_summary.write_tidy_result(
            ctx=self.ctx,
            app_name=app_name,
            stage="tidy-step",
            status="task_archived",
            exit_code=0,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
            verify_mode="full",
        )
        result_payload = tidy_shared.read_json_dict(result_path) or {}
        next_action = str(result_payload.get("next_action") or "").strip() or None
        active_batch_id = str((queue_head_after_close or {}).get("batch_id") or "").strip() or None
        state_path = batch_state.update_state(
            ctx=self.ctx,
            app_name=app_name,
            tidy_build_dir_name=tidy_build_dir_name,
            batch_id=active_batch_id,
            cleaned_task_ids=[closed_task_id],
            last_verify_success=True,
            extra_fields={
                "batch_id": active_batch_id,
                "queue_batch_id": active_batch_id,
                "last_tidy_step_ok": True,
                "last_tidy_step_task": {
                    "batch_id": closed_batch_id,
                    "task_id": closed_task_id,
                    "task_log": str(closed_task_path),
                    "recheck_log": str(recheck_log_path),
                },
                "queue_requires_reresolve": False,
                "historical_selection_stale": False,
                "reparse_required_reason": None,
                "next_queue_head": queue_head_after_close,
                "replacement_queue_head": None,
                "queue_transition_summary": transition_summary,
                "historical_batch": None,
                "next_action": next_action,
            },
        )
        print(f"--- tidy-step: queue snapshot updated -> {result_path}")
        print(f"--- tidy-step: batch state updated -> {state_path}")

    def _build_queue_head(self, tasks_dir: Path) -> dict | None:
        pending_logs = list_task_paths(tasks_dir)
        if not pending_logs:
            return None
        head_path = pending_logs[0]
        parsed = load_task_record(head_path)
        return {
            "task_id": parsed.task_id,
            "batch_id": parsed.batch_id,
            "queue_batch_id": parsed.batch_id,
            "source_file": parsed.source_file or str(head_path),
            "task_log": str(head_path),
            "checks": list(parsed.checks),
        }

    def _build_single_task_close_next_action(
        self,
        *,
        batch_id: str,
        task_id: str,
        queue_head: dict | None,
    ) -> str:
        next_action = (
            f"Single-task batch {batch_id}/task_{task_id} is closed. Stop here and "
            "re-resolve the current queue from tasks/ before continuing; do not keep using "
            f"{batch_id}/task_{task_id} as the active selection."
        )
        if queue_head is not None:
            next_action += (
                f" Current queue head: {queue_head.get('batch_id', '<BATCH_ID>')}/"
                f"task_{queue_head.get('task_id', '<TASK_ID>')} -> "
                f"{queue_head.get('task_log', '')}"
            )
        else:
            next_action += " No pending tasks remain in tasks/."
        return next_action

    def _build_close_transition_summary(
        self,
        *,
        batch_id: str,
        task_id: str,
        queue_head: dict | None,
    ) -> str:
        summary = f"Closed {batch_id}/task_{task_id}."
        if queue_head is not None:
            summary += (
                f" Current queue head is {queue_head.get('batch_id', '<BATCH_ID>')}/"
                f"task_{queue_head.get('task_id', '<TASK_ID>')} -> "
                f"{queue_head.get('task_log', '')}."
            )
        else:
            summary += " No pending tasks remain."
        return summary

    def _run_task_recheck(
        self,
        *,
        app_name: str,
        parsed,
        tidy_build_dir_name: str,
        source_scope: str | None,
    ) -> TaskRecheckResult:
        tidy_layout = self.ctx.get_tidy_layout(app_name, tidy_build_dir_name)
        build_dir = tidy_layout.root
        ensure_ret = tidy_refresh_runner.ensure_analysis_compile_db(
            ctx=self.ctx,
            app_name=app_name,
            build_dir=build_dir,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        log_path = (
            tidy_layout.automation_dir
            / f"{parsed.batch_id}_task_{parsed.task_id}_recheck.log"
        )
        if ensure_ret != 0:
            return TaskRecheckResult(
                ok=False,
                exit_code=ensure_ret,
                log_path=log_path,
                remaining_diagnostics=(),
            )

        source_files = self._collect_recheck_files(parsed)
        if not source_files:
            return TaskRecheckResult(
                ok=False,
                exit_code=1,
                log_path=log_path,
                remaining_diagnostics=(),
            )

        compile_db_dir = tidy_refresh_runner.analysis_compile_db.ensure_analysis_compile_db(
            build_dir
        )
        checks = [check for check in parsed.checks if check]
        header_filter = (self.ctx.config.tidy.header_filter_regex or "").strip()
        if not header_filter:
            header_filter = r"^(?!.*[\\/]_deps[\\/]).*"

        cmd = [
            "clang-tidy",
            "-p",
            str(compile_db_dir),
            f"-header-filter={header_filter}",
        ]
        if checks and all(str(check).startswith("clang-diagnostic-") for check in checks):
            cmd.append("--allow-no-checks")
        if checks:
            cmd.append(f"-checks=-*,{','.join(checks)}")
        cmd.append("--quiet")
        cmd.extend(str(path) for path in source_files)

        recheck_ret = run_command(
            cmd,
            cwd=self.ctx.repo_root,
            env=self.ctx.setup_env(),
            log_file=log_path,
            output_mode="quiet",
        )
        try:
            log_lines = log_path.read_text(encoding="utf-8", errors="replace").splitlines()
        except OSError:
            log_lines = []
        diagnostics = log_parser.extract_diagnostics(log_lines)
        remaining = tuple(self._match_remaining_diagnostics(parsed, diagnostics))
        return TaskRecheckResult(
            ok=(recheck_ret == 0 and not remaining),
            exit_code=recheck_ret,
            log_path=log_path,
            remaining_diagnostics=remaining,
        )

    def _collect_recheck_files(self, parsed) -> list[Path]:
        files: list[Path] = []
        seen: set[str] = set()
        raw_paths = [parsed.source_file, *(diagnostic.file for diagnostic in parsed.diagnostics)]
        for raw_path in raw_paths:
            text = str(raw_path or "").strip()
            if not text:
                continue
            path = Path(text)
            normalized = self._path_key(path)
            if normalized in seen:
                continue
            seen.add(normalized)
            files.append(path)
        return files

    def _match_remaining_diagnostics(
        self,
        parsed,
        diagnostics: list[dict],
    ) -> list[dict]:
        expected_keys = {
            (
                self._path_key(diagnostic.file or parsed.source_file),
                int(diagnostic.line),
                int(diagnostic.col),
                diagnostic.check,
            )
            for diagnostic in parsed.diagnostics
        }
        remaining: list[dict] = []
        for diagnostic in diagnostics:
            current_key = (
                self._path_key(diagnostic.get("file", "") or parsed.source_file),
                int(diagnostic.get("line", 0)),
                int(diagnostic.get("col", 0)),
                str(diagnostic.get("check", "")).strip(),
            )
            if current_key in expected_keys:
                remaining.append(diagnostic)
        return remaining

    @staticmethod
    def _path_key(path_like) -> str:
        return str(Path(str(path_like))).replace("\\", "/").lower()

    @staticmethod
    def _can_continue_after_fix_failures(fix_result) -> bool:
        allowed_rename_failure_markers = (
            "Cannot rename symbol: there is no symbol at the given location",
            "Cannot rename symbol: symbol is not a supported kind",
        )
        if fix_result.failed <= 0:
            return True
        failed_actions = [
            action for action in fix_result.actions if action.status == "failed"
        ]
        if not failed_actions:
            return False
        for action in failed_actions:
            reason = str(action.reason or "")
            if action.kind != "rename":
                return False
            if not any(marker in reason for marker in allowed_rename_failure_markers):
                return False
        return True
