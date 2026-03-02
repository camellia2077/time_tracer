from ...core.context import Context
from ...core.executor import run_command
from ..cmd_build import BuildCommand


class LintCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def _resolve_backend(self, app_name: str) -> str:
        app = self.ctx.get_app_metadata(app_name)
        backend = (getattr(app, "backend", "cmake") or "cmake").strip().lower()
        if backend in {"cmake", "gradle", "cargo"}:
            return backend
        return "cmake"

    def execute(
        self,
        app_name: str,
        extra_args: list[str] | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
    ) -> int:
        backend = self._resolve_backend(app_name)
        filtered_args = [arg for arg in (extra_args or []) if arg != "--"]

        if backend == "cargo":
            command = ["cargo", "clippy", "--all-targets", "--all-features", *filtered_args]
            print(f"--- lint: start ({app_name}), backend=cargo")
            ret = run_command(
                command,
                cwd=self.ctx.get_app_dir(app_name),
                env=self.ctx.setup_env(),
            )
            if ret == 0:
                print(f"--- lint: done ({app_name})")
                return 0
            print(f"--- lint: failed ({app_name}), exit={ret}")
            return int(ret)

        if backend == "cmake":
            build_cmd = BuildCommand(self.ctx)
            print(f"--- lint: backend=cmake, redirect to tidy ({app_name})")
            return build_cmd.build(
                app_name=app_name,
                tidy=True,
                extra_args=filtered_args,
                cmake_args=None,
                build_dir_name=build_dir_name,
                profile_name=profile_name,
                kill_build_procs=False,
            )

        print(f"Error: lint command does not support backend `{backend}` for app `{app_name}`.")
        return 2
