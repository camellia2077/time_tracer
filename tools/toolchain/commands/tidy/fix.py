from ...core.context import Context
from ...core.executor import run_command
from . import workspace as tidy_workspace


class TidyFixCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        limit: int | None = None,
        jobs: int | None = None,
        keep_going: bool | None = None,
        source_scope: str | None = None,
        tidy_build_dir_name: str | None = None,
    ) -> int:
        from ..cmd_build import BuildCommand

        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        build_dir = self.ctx.get_tidy_dir(app_name, workspace.build_dir_name)

        if not (build_dir / "CMakeCache.txt").exists():
            print(
                f"--- tidy-fix: {workspace.build_dir_name} is not configured. "
                "Running auto-configure..."
            )
            ret = BuildCommand(self.ctx).configure(
                app_name=app_name,
                tidy=True,
                source_scope=workspace.source_scope,
                build_dir_name=workspace.build_dir_name,
            )
            if ret != 0:
                print("--- tidy-fix: auto-configure failed.")
                return ret

        configured_limit = self.ctx.config.tidy.fix_limit
        configured_jobs = self.ctx.config.tidy.jobs
        configured_keep_going = self.ctx.config.tidy.keep_going

        effective_limit = configured_limit if limit is None else limit
        effective_jobs = configured_jobs if jobs is None else jobs
        effective_keep_going = configured_keep_going if keep_going is None else keep_going

        target = "tidy-fix"
        if effective_limit and effective_limit > 0:
            target = f"tidy_fix_step_{effective_limit}"

        print(f"--- tidy-fix: running target `{target}`")
        cmd = ["cmake", "--build", str(build_dir), "--target", target]
        if effective_jobs and effective_jobs > 0:
            cmd += [f"-j{effective_jobs}"]
        else:
            cmd += ["-j"]
        if effective_keep_going:
            cmd += ["--", "-k", "0"]

        ret = run_command(cmd, env=self.ctx.setup_env())
        if ret != 0:
            print(f"--- tidy-fix finished with code {ret}.")
        return ret
