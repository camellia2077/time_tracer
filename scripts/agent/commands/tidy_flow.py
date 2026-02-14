import json
import re
from datetime import datetime, timezone
from pathlib import Path

from ..core.context import Context
from ..core.executor import run_command
from ..services import log_parser
from .build import BuildCommand
from .clean import CleanCommand
from .rename import RenameCommand
from .tidy import TidyCommand
from .tidy_fix import TidyFixCommand
from .tidy_loop import TidyLoopCommand

TASK_ID_PATTERN = re.compile(r"task_(\d+)\.log$")


class TidyFlowCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        process_all: bool = False,
        n: int = 1,
        resume: bool = False,
        test_every: int = 3,
        concise: bool = False,
        jobs: int | None = None,
        parse_workers: int | None = None,
        keep_going: bool | None = None,
        run_tidy_fix: bool | None = None,
        tidy_fix_limit: int | None = None,
        kill_build_procs: bool = False,
    ) -> int:
        if not process_all and n <= 0:
            print("--- tidy-flow: n <= 0, nothing to do.")
            return 0

        app_dir = self.ctx.get_app_dir(app_name)
        build_tidy_dir = app_dir / "build_tidy"
        tasks_dir = build_tidy_dir / "tasks"
        state_path = build_tidy_dir / "flow_state.json"
        effective_test_every = max(1, test_every)
        effective_n = n if n is not None else 1
        effective_keep_going = (
            self.ctx.config.tidy.keep_going if keep_going is None else keep_going
        )
        effective_run_tidy_fix = (
            self.ctx.config.tidy.run_fix_before_tidy
            if run_tidy_fix is None
            else run_tidy_fix
        )
        effective_tidy_fix_limit = (
            self.ctx.config.tidy.fix_limit
            if tidy_fix_limit is None
            else tidy_fix_limit
        )

        state = self._new_state(
            app_name=app_name,
            process_all=process_all,
            n=effective_n,
            resume=resume,
            test_every=effective_test_every,
            concise=concise,
            jobs=jobs,
            parse_workers=parse_workers,
            keep_going=effective_keep_going,
            run_tidy_fix=effective_run_tidy_fix,
            tidy_fix_limit=effective_tidy_fix_limit,
            kill_build_procs=kill_build_procs,
            state_path=state_path,
        )

        self._set_phase(state, "prepare_tasks")
        task_log_exists = self._has_task_logs(tasks_dir)
        if resume and task_log_exists:
            print(
                "--- tidy-flow: resume enabled and existing task logs found, "
                "skip tidy-fix/tidy generation."
            )
            self._set_step(state, "prepare_tasks", "skipped")
        else:
            tidy_fix_ret = None
            if effective_run_tidy_fix:
                print("--- tidy-flow: running pre-pass tidy-fix...")
                tidy_fix_ret = TidyFixCommand(self.ctx).execute(
                    app_name=app_name,
                    limit=effective_tidy_fix_limit,
                    jobs=jobs,
                    keep_going=effective_keep_going,
                )
                state["tidy_fix_exit_code"] = tidy_fix_ret
                if tidy_fix_ret != 0:
                    print(
                        "--- tidy-flow: tidy-fix returned non-zero; "
                        "continue to tidy task generation."
                    )

            print("--- tidy-flow: generating/refreshing tidy tasks...")
            tidy_ret = TidyCommand(self.ctx).execute(
                app_name=app_name,
                extra_args=[],
                jobs=jobs,
                parse_workers=parse_workers,
                keep_going=effective_keep_going,
            )
            if tidy_ret != 0:
                self._set_step(state, "prepare_tasks", "failed", tidy_ret)
                return self._finish(
                    state=state,
                    state_path=state_path,
                    exit_code=tidy_ret,
                    pending_task_ids=self._list_task_ids(tasks_dir),
                )
            self._set_step(state, "prepare_tasks", "done", 0)

        self._set_phase(state, "rename")
        rename_cmd = RenameCommand(self.ctx)
        rename_plan_ret = rename_cmd.plan(app_name)
        if rename_plan_ret != 0:
            self._set_step(state, "rename", "failed", rename_plan_ret)
            return self._finish(
                state=state,
                state_path=state_path,
                exit_code=rename_plan_ret,
                pending_task_ids=self._list_task_ids(tasks_dir),
            )

        rename_candidates = self._count_rename_candidates(build_tidy_dir)
        state["rename_candidates"] = rename_candidates
        if rename_candidates > 0:
            rename_apply_ret = rename_cmd.apply(app_name, strict=True)
            if rename_apply_ret != 0:
                self._set_step(state, "rename", "failed", rename_apply_ret)
                return self._finish(
                    state=state,
                    state_path=state_path,
                    exit_code=rename_apply_ret,
                    pending_task_ids=self._list_task_ids(tasks_dir),
                )

            rename_audit_ret = rename_cmd.audit(app_name, strict=True)
            if rename_audit_ret != 0:
                self._set_step(state, "rename", "failed", rename_audit_ret)
                return self._finish(
                    state=state,
                    state_path=state_path,
                    exit_code=rename_audit_ret,
                    pending_task_ids=self._list_task_ids(tasks_dir),
                )
        else:
            print("--- tidy-flow: no rename candidates, skip rename-apply/rename-audit.")
        self._set_step(state, "rename", "done", 0)

        self._set_phase(state, "verify")
        build_cmd = BuildCommand(self.ctx)
        configure_ret = build_cmd.configure(
            app_name=app_name,
            tidy=False,
            extra_args=None,
            kill_build_procs=kill_build_procs,
        )
        if configure_ret != 0:
            self._set_step(state, "verify", "failed", configure_ret)
            return self._finish(
                state=state,
                state_path=state_path,
                exit_code=configure_ret,
                pending_task_ids=self._list_task_ids(tasks_dir),
            )

        build_ret = build_cmd.build(
            app_name=app_name,
            tidy=False,
            extra_args=None,
            kill_build_procs=kill_build_procs,
        )
        if build_ret != 0:
            self._set_step(state, "verify", "failed", build_ret)
            return self._finish(
                state=state,
                state_path=state_path,
                exit_code=build_ret,
                pending_task_ids=self._list_task_ids(tasks_dir),
            )

        suite_ret = self._run_suite_verify(app_name=app_name, concise=concise)
        if suite_ret != 0:
            self._set_step(state, "verify", "failed", suite_ret)
            return self._finish(
                state=state,
                state_path=state_path,
                exit_code=suite_ret,
                pending_task_ids=self._list_task_ids(tasks_dir),
            )
        self._set_step(state, "verify", "done", 0)

        self._set_phase(state, "loop")
        tidy_loop_cmd = TidyLoopCommand(self.ctx)
        loop_ret = tidy_loop_cmd.execute(
            app_name=app_name,
            n=effective_n,
            process_all=process_all,
            test_every=effective_test_every,
            concise=concise,
            kill_build_procs=kill_build_procs,
        )
        if loop_ret not in (0, 2):
            self._set_step(state, "loop", "failed", loop_ret)
            return self._finish(
                state=state,
                state_path=state_path,
                exit_code=loop_ret,
                pending_task_ids=self._list_task_ids(tasks_dir),
            )
        self._set_step(state, "loop", "done", loop_ret)

        self._set_phase(state, "clean")
        cleaned_empty_task_ids = self._clean_empty_tasks(app_name, tasks_dir)
        if cleaned_empty_task_ids:
            print(
                f"--- tidy-flow: cleaned empty task logs: "
                f"{', '.join(cleaned_empty_task_ids)}"
            )
        state["cleaned_empty_task_ids"] = cleaned_empty_task_ids
        self._set_step(state, "clean", "done", 0)

        pending_task_ids = self._list_task_ids(tasks_dir)
        blocked_task_id = pending_task_ids[0] if pending_task_ids else None
        if loop_ret == 2:
            final_exit_code = 2
        elif process_all and pending_task_ids:
            final_exit_code = 2
        else:
            final_exit_code = 0

        return self._finish(
            state=state,
            state_path=state_path,
            exit_code=final_exit_code,
            pending_task_ids=pending_task_ids,
            blocked_task_id=blocked_task_id,
        )

    def _new_state(
        self,
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
        kill_build_procs: bool,
        state_path: Path,
    ) -> dict:
        mode = "all" if process_all else f"n:{n}"
        now = self._utc_now_iso()
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
        self._write_state(state_path, state)
        return state

    def _set_phase(self, state: dict, phase_name: str) -> None:
        state["phase"] = phase_name
        state["updated_at"] = self._utc_now_iso()

    def _set_step(
        self, state: dict, step_name: str, status: str, exit_code: int | None = None
    ) -> None:
        state["steps"][step_name]["status"] = status
        state["steps"][step_name]["exit_code"] = exit_code
        state["updated_at"] = self._utc_now_iso()

    def _finish(
        self,
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
        state["updated_at"] = self._utc_now_iso()
        self._write_state(state_path, state)

        print(
            f"--- tidy-flow summary: exit={exit_code}, pending={len(pending_task_ids)}, "
            f"blocked={blocked_task_id if blocked_task_id else '-'}"
        )
        print(f"--- tidy-flow state: {state_path}")
        return exit_code

    def _write_state(self, state_path: Path, state: dict) -> None:
        state_path.parent.mkdir(parents=True, exist_ok=True)
        state_path.write_text(
            json.dumps(state, ensure_ascii=False, indent=2), encoding="utf-8"
        )

    def _count_rename_candidates(self, build_tidy_dir: Path) -> int:
        candidates_path = build_tidy_dir / "rename" / "rename_candidates.json"
        if not candidates_path.exists():
            return 0
        try:
            payload = json.loads(candidates_path.read_text(encoding="utf-8"))
        except json.JSONDecodeError:
            return 0
        candidates = payload.get("candidates", [])
        if isinstance(candidates, list):
            return len(candidates)
        return 0

    def _run_suite_verify(self, app_name: str, concise: bool) -> int:
        cmd = [
            "python",
            "test/run.py",
            "--suite",
            app_name,
            "--agent",
            "--build-dir",
            "build_fast",
        ]
        if concise:
            cmd.append("--concise")
        return run_command(
            cmd,
            cwd=self.ctx.repo_root,
            env=self.ctx.setup_env(),
        )

    def _has_task_logs(self, tasks_dir: Path) -> bool:
        return any(tasks_dir.rglob("task_*.log"))

    def _list_task_paths(self, tasks_dir: Path) -> list[Path]:
        task_paths = list(tasks_dir.rglob("task_*.log"))
        task_paths.sort(key=self._task_sort_key)
        return task_paths

    def _list_task_ids(self, tasks_dir: Path) -> list[str]:
        task_ids: list[str] = []
        for task_path in self._list_task_paths(tasks_dir):
            task_id = self._task_id(task_path)
            if task_id:
                task_ids.append(task_id)
        return task_ids

    def _clean_empty_tasks(self, app_name: str, tasks_dir: Path) -> list[str]:
        if not tasks_dir.exists():
            return []

        empty_task_ids: list[str] = []
        for task_path in self._list_task_paths(tasks_dir):
            content = task_path.read_text(encoding="utf-8", errors="replace")
            diagnostics = log_parser.extract_diagnostics(content.splitlines())
            if diagnostics:
                continue
            task_id = self._task_id(task_path)
            if task_id:
                empty_task_ids.append(task_id)

        if not empty_task_ids:
            return []

        clean_ret = CleanCommand(self.ctx).execute(app_name, empty_task_ids)
        if clean_ret != 0:
            return []
        return empty_task_ids

    def _task_sort_key(self, task_path: Path) -> tuple[int, str]:
        match = TASK_ID_PATTERN.match(task_path.name)
        if not match:
            return 10**9, task_path.name
        return int(match.group(1)), task_path.name

    def _task_id(self, task_path: Path) -> str | None:
        match = TASK_ID_PATTERN.match(task_path.name)
        if not match:
            return None
        return match.group(1).zfill(3)

    def _utc_now_iso(self) -> str:
        return datetime.now(timezone.utc).isoformat()
