import subprocess

from ..core.context import Context
from .build import BuildCommand


class FormatCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(self, app_name: str, extra_args: list | None = None) -> int:
        build_dir = self.ctx.get_app_dir(app_name) / "build_agent"
        if not (build_dir / "CMakeCache.txt").exists():
            print("--- format: build_agent is not configured. Running configure...")
            build_cmd = BuildCommand(self.ctx)
            ret = build_cmd.configure(
                app_name=app_name,
                tidy=False,
                extra_args=None,
                kill_build_procs=False,
            )
            if ret != 0:
                return ret

        filtered_args = [arg for arg in (extra_args or []) if arg != "--"]
        command = ["cmake", "--build", str(build_dir), "--target", "format"] + filtered_args
        print(f"--- format: start ({app_name})")
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
