from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

from ...core.executor import run_command
from ...services import log_parser
from ...core.context import Context
from ..cmd_quality.verify import VerifyCommand
from ..shared import tidy as tidy_shared
from .batch import TidyBatchCommand
from .clean import CleanCommand
from .refresh_internal import refresh_runner as tidy_refresh_runner
from .task_auto_fix import run_task_auto_fix
from .task_log import list_task_paths, parse_task_log, resolve_task_log_path
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
        build_tidy_dir = tidy_layout.root
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

        print("--- tidy-step: running task-scope verify...")
        verify_ret = VerifyCommand(self.ctx).execute(
            app_name=app_name,
            tidy=False,
            build_dir_name=verify_build_dir_name,
            profile_name=profile_name,
            concise=concise,
            kill_build_procs=kill_build_procs,
            verify_scope="task",
        )
        if verify_ret != 0:
            print("--- tidy-step: task-scope verify failed.")
            return verify_ret

        print("--- tidy-step: running task-scope clang-tidy re-check...")
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

        if len(batch_tasks_before) == 1:
            print(
                f"--- tidy-step: {parsed.batch_id} contains a single task. "
                "Running tidy-batch --preset sop..."
            )
            return TidyBatchCommand(self.ctx).execute(
                app_name=app_name,
                batch_id=parsed.batch_id,
                strict_clean=True,
                full_every=3,
                keep_going=True,
                run_verify=True,
                verify_build_dir_name=verify_build_dir_name,
                tidy_build_dir_name=workspace.build_dir_name,
                source_scope=workspace.source_scope,
                profile_name=profile_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
            )

        next_action = (
            "Next: run `python tools/run.py tidy-batch --app "
            f"{app_name}"
        )
        if workspace.source_scope:
            next_action += f" --source-scope {workspace.source_scope}"
        if workspace.build_dir_name:
            next_action += f" --tidy-build-dir {workspace.build_dir_name}"
        next_action += f" --batch-id {parsed.batch_id} --preset sop`"
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
                "next_action": next_action,
            },
        )
        print(f"--- tidy-step: step state -> {step_state_path}")
        return 0

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
            if "Cannot rename symbol: there is no symbol at the given location" not in reason:
                return False
        return True
