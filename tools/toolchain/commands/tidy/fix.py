from ...core.context import Context
from ...core.executor import run_command
from . import invoker as tidy_invoker, workspace as tidy_workspace


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
        config_file: str | None = None,
        strict_config: bool = False,
    ) -> int:
        workspace = tidy_workspace.resolve_workspace(
            self.ctx,
            build_dir_name=tidy_build_dir_name,
            source_scope=source_scope,
        )
        build_dir = self.ctx.get_tidy_dir(app_name, workspace.build_dir_name)

        ret, _, _ = tidy_invoker.ensure_configured(
            self.ctx,
            app_name,
            build_dir,
            source_scope=workspace.source_scope,
            config_file=config_file,
            strict_config=strict_config,
            build_dir_name=workspace.build_dir_name,
        )
        if ret != 0:
            print("--- tidy-fix: auto-configure failed.")
            return ret

        configured_limit = self.ctx.config.tidy.fix_limit
        configured_keep_going = self.ctx.config.tidy.keep_going

        effective_limit = configured_limit if limit is None else limit
        effective_jobs = tidy_invoker.resolve_effective_tidy_jobs(
            self.ctx,
            jobs,
            mode="task_batch",
        )
        effective_keep_going = configured_keep_going if keep_going is None else keep_going

        target = "tidy-fix"
        if effective_limit and effective_limit > 0:
            target = f"tidy_fix_step_{effective_limit}"

        print(f"--- tidy-fix: running target `{target}`")
        cmd = ["cmake", "--build", str(build_dir), "--target", target]
        cmd += [f"-j{effective_jobs}"]
        if effective_keep_going:
            cmd += ["--", "-k", "0"]

        ret = run_command(cmd, env=self.ctx.setup_env())
        if ret != 0:
            print(f"--- tidy-fix finished with code {ret}.")
        return ret
