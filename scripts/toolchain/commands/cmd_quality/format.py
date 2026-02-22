import subprocess

from ...core.context import Context
from ..cmd_build import BuildCommand


class FormatCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        app_name: str,
        extra_args: list | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
    ) -> int:
        build_cmd = BuildCommand(self.ctx)
        resolved_build_dir_name = build_cmd.resolve_build_dir_name(
            tidy=False,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            app_name=app_name,
        )
        build_dir = self.ctx.get_app_dir(app_name) / resolved_build_dir_name
        if not (build_dir / "CMakeCache.txt").exists():
            print(f"--- format: {resolved_build_dir_name} is not configured. Running configure...")
            ret = build_cmd.configure(
                app_name=app_name,
                tidy=False,
                extra_args=None,
                build_dir_name=resolved_build_dir_name,
                profile_name=profile_name,
                kill_build_procs=False,
            )
            if ret != 0:
                return ret

        filtered_args = [arg for arg in (extra_args or []) if arg != "--"]
        has_target_override = "--target" in filtered_args
        default_target = "format_all" if app_name == "tracer_windows_cli" else "format"
        command = ["cmake", "--build", str(build_dir)]
        if not has_target_override:
            command += ["--target", default_target]
        command += filtered_args
        print(
            f"--- format: start ({app_name}), target={default_target if not has_target_override else 'custom'}"
        )
        completed = subprocess.run(
            command,
            cwd=self.ctx.repo_root,
            env=self.ctx.setup_env(),
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
            check=False,
        )
        if completed.returncode == 0:
            print(f"--- format: done ({app_name})")
            return 0

        print(f"--- format: failed ({app_name}), exit={completed.returncode}")
        return int(completed.returncode)
