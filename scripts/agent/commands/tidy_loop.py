import re
from pathlib import Path

from ..core.context import Context
from ..core.executor import run_command
from ..services import log_parser
from .build import BuildCommand
from .clean import CleanCommand
from .rename import RenameCommand

TASK_ID_PATTERN = re.compile(r"task_(\d+)\.log$")


class TidyLoopCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        n: int = 1,
        process_all: bool = False,
        test_every: int = 1,
        concise: bool = False,
        kill_build_procs: bool = False,
    ) -> int:
        if not process_all and n <= 0:
            print("--- tidy-loop: n <= 0, nothing to do.")
            return 0

        effective_test_every = max(1, test_every)
        app_dir = self.ctx.get_app_dir(app_name)
        tasks_dir = app_dir / "build_tidy" / "tasks"
        if not tasks_dir.exists():
            print(f"--- tidy-loop: tasks directory not found: {tasks_dir}")
            return 1

        clean_cmd = CleanCommand(self.ctx)
        rename_cmd = RenameCommand(self.ctx)

        cleaned = 0
        rename_done = False
        blocked_by_manual_task = False

        while True:
            if not process_all and cleaned >= n:
                break

            task_path = self._next_task_path(tasks_dir)
            if not task_path:
                break

            task_id = self._task_id(task_path)
            task_kind = self._classify_task(task_path)
            print(f"--- tidy-loop: selected task {task_id} ({task_kind})")

            if task_kind == "rename_only":
                if not rename_done:
                    print("--- tidy-loop: applying rename pipeline once...")
                    ret = rename_cmd.plan(app_name)
                    if ret != 0:
                        return ret
                    ret = rename_cmd.apply(app_name, strict=True)
                    if ret != 0:
                        return ret
                    ret = rename_cmd.audit(app_name, strict=True)
                    if ret != 0:
                        return ret
                    rename_done = True

                ret = clean_cmd.execute(app_name, [task_id])
                if ret != 0:
                    return ret
                cleaned += 1

            elif task_kind == "empty":
                ret = clean_cmd.execute(app_name, [task_id])
                if ret != 0:
                    return ret
                cleaned += 1

            else:
                blocked_by_manual_task = True
                print(
                    f"--- tidy-loop: task {task_id} requires manual fix. "
                    "Stopping loop."
                )
                break

            if cleaned > 0 and cleaned % effective_test_every == 0:
                ret = self._verify(
                    app_name=app_name,
                    concise=concise,
                    kill_build_procs=kill_build_procs,
                )
                if ret != 0:
                    return ret

        if cleaned > 0 and cleaned % effective_test_every != 0:
            ret = self._verify(
                app_name=app_name,
                concise=concise,
                kill_build_procs=kill_build_procs,
            )
            if ret != 0:
                return ret

        print(
            f"--- tidy-loop summary: cleaned={cleaned}, target={'all' if process_all else n}, "
            f"test_every={effective_test_every}"
        )

        if blocked_by_manual_task and (process_all or cleaned < n):
            return 2
        return 0

    def _next_task_path(self, tasks_dir: Path) -> Path | None:
        task_files = list(tasks_dir.rglob("task_*.log"))
        if not task_files:
            return None
        task_files.sort(key=self._task_sort_key)
        return task_files[0]

    def _task_sort_key(self, path: Path) -> tuple[int, str]:
        match = TASK_ID_PATTERN.match(path.name)
        if not match:
            return 10**9, path.name
        return int(match.group(1)), path.name

    def _task_id(self, path: Path) -> str:
        match = TASK_ID_PATTERN.match(path.name)
        if not match:
            return path.stem
        return match.group(1).zfill(3)

    def _classify_task(self, task_path: Path) -> str:
        content = task_path.read_text(encoding="utf-8", errors="replace")
        diagnostics = log_parser.extract_diagnostics(content.splitlines())
        if not diagnostics:
            return "empty"

        checks = {
            diag.get("check", "").strip()
            for diag in diagnostics
            if diag.get("check", "").strip()
        }
        if not checks:
            return "manual"

        rename_check = self.ctx.config.rename.check_name
        if checks == {rename_check}:
            return "rename_only"
        return "manual"

    def _ensure_build_configured(
        self, app_name: str, kill_build_procs: bool
    ) -> int:
        app_dir = self.ctx.get_app_dir(app_name)
        build_dir = app_dir / "build_fast"
        if (build_dir / "CMakeCache.txt").exists():
            return 0

        print("--- tidy-loop: build_fast is not configured. Running configure...")
        builder = BuildCommand(self.ctx)
        return builder.configure(
            app_name=app_name,
            tidy=False,
            extra_args=None,
            kill_build_procs=kill_build_procs,
        )

    def _verify(self, app_name: str, concise: bool, kill_build_procs: bool) -> int:
        ret = self._ensure_build_configured(
            app_name=app_name, kill_build_procs=kill_build_procs
        )
        if ret != 0:
            return ret

        builder = BuildCommand(self.ctx)
        ret = builder.build(
            app_name=app_name,
            tidy=False,
            extra_args=None,
            kill_build_procs=kill_build_procs,
        )
        if ret != 0:
            return ret

        test_cmd = [
            "python",
            "test/run.py",
            "--suite",
            app_name,
            "--agent",
            "--build-dir",
            "build_fast",
        ]
        if concise:
            test_cmd.append("--concise")
        return run_command(
            test_cmd,
            cwd=self.ctx.repo_root,
            env=self.ctx.setup_env(),
        )
