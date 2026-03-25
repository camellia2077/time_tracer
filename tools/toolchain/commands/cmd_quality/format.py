import subprocess
from pathlib import Path
import shutil

from ...core.context import Context
from ...core.executor import run_command
from ..cmd_build import BuildCommand


_FORMATTABLE_SUFFIXES = {
    ".c",
    ".cc",
    ".cpp",
    ".cxx",
    ".h",
    ".hh",
    ".hpp",
    ".hxx",
    ".inl",
    ".inc",
    ".ipp",
    ".ixx",
    ".cppm",
}
_SKIP_DIR_NAMES = {
    ".git",
    ".hg",
    ".svn",
    ".vs",
    ".idea",
    ".vscode",
    "__pycache__",
    "out",
}


class FormatCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def _resolve_backend(self, app_name: str) -> str:
        app = self.ctx.get_app_metadata(app_name)
        backend = (getattr(app, "backend", "cmake") or "cmake").strip().lower()
        if backend in {"cmake", "gradle", "cargo"}:
            return backend
        return "cmake"

    def _resolve_clang_format(self) -> str | None:
        env = self.ctx.setup_env()
        return shutil.which("clang-format", path=env.get("PATH"))

    @staticmethod
    def _should_skip_path(path: Path) -> bool:
        lowered_parts = {part.lower() for part in path.parts}
        if lowered_parts & _SKIP_DIR_NAMES:
            return True
        return any(part.lower().startswith("build") for part in path.parts)

    def _collect_formattable_files(self, raw_paths: list[str]) -> list[Path]:
        files: list[Path] = []
        seen: set[Path] = set()

        for raw_path in raw_paths:
            candidate = Path(raw_path)
            resolved = candidate if candidate.is_absolute() else (self.ctx.repo_root / candidate)
            resolved = resolved.resolve()
            if not resolved.exists():
                raise FileNotFoundError(f"format path does not exist: {raw_path}")

            if resolved.is_file():
                if resolved.suffix.lower() in _FORMATTABLE_SUFFIXES and not self._should_skip_path(resolved):
                    if resolved not in seen:
                        seen.add(resolved)
                        files.append(resolved)
                continue

            for child in resolved.rglob("*"):
                if not child.is_file():
                    continue
                if child.suffix.lower() not in _FORMATTABLE_SUFFIXES:
                    continue
                if self._should_skip_path(child):
                    continue
                if child not in seen:
                    seen.add(child)
                    files.append(child)

        return sorted(files)

    def _run_path_mode(
        self,
        *,
        raw_paths: list[str],
        check_only: bool,
    ) -> int:
        clang_format = self._resolve_clang_format()
        if not clang_format:
            print("--- format: failed (paths), clang-format was not found on PATH")
            return 1

        try:
            files = self._collect_formattable_files(raw_paths)
        except FileNotFoundError as exc:
            print(f"--- format: failed (paths), {exc}")
            return 1

        if not files:
            print("--- format: no matching C/C++ files found for requested paths.")
            return 0

        mode_label = "check" if check_only else "apply"
        print(f"--- format: start (paths), mode={mode_label}, files={len(files)}")
        env = self.ctx.setup_env()
        for index, file_path in enumerate(files, start=1):
            command = [clang_format, "-style=file"]
            if check_only:
                command.extend(["--dry-run", "--Werror"])
            else:
                command.append("-i")
            command.append(str(file_path))
            print(f"[{index}/{len(files)}] {'Checking' if check_only else 'Formatting'}: {file_path}")
            result = subprocess.run(
                command,
                cwd=self.ctx.repo_root,
                env=env,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding="utf-8",
                errors="replace",
                check=False,
            )
            if result.returncode != 0:
                output = (result.stdout or "").rstrip()
                if output:
                    print(output)
                print(f"--- format: failed (paths), exit={result.returncode}")
                return int(result.returncode)

        print("--- format: done (paths)")
        return 0

    def execute(
        self,
        app_name: str | None = None,
        raw_paths: list[str] | None = None,
        check_only: bool = False,
        extra_args: list | None = None,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
    ) -> int:
        requested_paths = [str(path) for path in (raw_paths or [])]
        if app_name and requested_paths:
            print("--- format: failed, use either --app or --paths, not both.")
            return 2
        if not app_name and not requested_paths:
            print("--- format: failed, provide --app or --paths.")
            return 2

        filtered_args = [arg for arg in (extra_args or []) if arg != "--"]
        if requested_paths:
            if filtered_args:
                print("--- format: failed, extra build target arguments are not supported with --paths.")
                return 2
            return self._run_path_mode(raw_paths=requested_paths, check_only=check_only)

        assert app_name is not None
        backend = self._resolve_backend(app_name)
        if backend == "cargo":
            command = ["cargo", "fmt"]
            if check_only:
                command.append("--check")
            command.extend(filtered_args)
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
        default_target = "check-format" if check_only else "format"
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
