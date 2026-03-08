from ...core.context import Context
from ...core.executor import run_command
from ..cmd_build import BuildCommand


class FormatCommand:
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
        extra_args: list | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
    ) -> int:
        backend = self._resolve_backend(app_name)
        filtered_args = [arg for arg in (extra_args or []) if arg != "--"]
        if backend == "cargo":
            command = ["cargo", "fmt", *filtered_args]
            print(f"--- format: start ({app_name}), backend=cargo")
            ret = run_command(
                command,
                cwd=self.ctx.get_app_dir(app_name),
                env=self.ctx.setup_env(),
            )
            if ret == 0:
                print(f"--- format: done ({app_name})")
                return 0
            print(f"--- format: failed ({app_name}), exit={ret}")
            return int(ret)

        build_cmd = BuildCommand(self.ctx)
        resolved_build_dir_name = build_cmd.resolve_build_dir_name(
            tidy=False,
            build_dir_name=build_dir_name,
            profile_name=profile_name,
            app_name=app_name,
        )
        build_dir = self.ctx.get_build_dir(app_name, resolved_build_dir_name)
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

        has_target_override = "--target" in filtered_args
        default_target = "format"
        command = ["cmake", "--build", str(build_dir)]
        if not has_target_override:
            command += ["--target", default_target]
        command += filtered_args
        print(
            f"--- format: start ({app_name}), target={default_target if not has_target_override else 'custom'}"
        )
        ret = run_command(
            command,
            cwd=self.ctx.repo_root,
            env=self.ctx.setup_env(),
        )
        if ret == 0:
            print(f"--- format: done ({app_name})")
            return 0

        print(f"--- format: failed ({app_name}), exit={ret}")
        return int(ret)
