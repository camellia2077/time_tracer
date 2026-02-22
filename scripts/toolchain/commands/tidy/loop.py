from ...core.context import Context
from ..cmd_rename import RenameCommand
from . import loop_tasks as tidy_loop_tasks, loop_verify as tidy_loop_verify
from .clean import CleanCommand


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
                print(f"--- tidy-loop: task {task_id} requires manual fix. Stopping loop.")
                break

            if cleaned > 0 and cleaned % effective_test_every == 0:
                ret = tidy_loop_verify.verify_loop(
                    ctx=self.ctx,
                    app_name=app_name,
                    concise=concise,
                    kill_build_procs=kill_build_procs,
                )
                if ret != 0:
                    return ret

        if cleaned > 0 and cleaned % effective_test_every != 0:
            ret = tidy_loop_verify.verify_loop(
                ctx=self.ctx,
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

    def _next_task_path(self, tasks_dir):
        return tidy_loop_tasks.next_task_path(tasks_dir)

    def _task_id(self, path):
        return tidy_loop_tasks.task_id(path)

    def _classify_task(self, task_path):
        return tidy_loop_tasks.classify_task(self.ctx, task_path)
