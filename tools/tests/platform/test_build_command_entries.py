import io
import sys
from contextlib import redirect_stdout
from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[3]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.cmd_build import command_entries  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402


def _write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


class _FakeBuildCommand:
    def __init__(self, ctx: Context, backend: str):
        self.ctx = ctx
        self._backend = backend

    def _resolve_backend(self, app_name: str) -> str:
        _ = app_name
        return self._backend

    def resolve_build_dir_name(
        self,
        tidy: bool,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
        app_name: str | None = None,
    ) -> str:
        _ = tidy, profile_name, app_name
        return build_dir_name or "build_fast"

    def _sync_platform_config_if_needed(self, app_name: str) -> int:
        _ = app_name
        return 0

    def _is_configured(self, app_name: str, tidy: bool, build_dir_name: str | None, profile_name: str | None) -> bool:
        _ = app_name, tidy, build_dir_name, profile_name
        return True

    def _needs_windows_config_reconfigure(self, app_name: str, build_dir: Path) -> bool:
        _ = app_name, build_dir
        return False

    def configure(self, **kwargs) -> int:
        _ = kwargs
        return 0

    def _sync_windows_runtime_config_copy_if_needed(self, app_name: str, build_dir_name: str) -> int:
        _ = app_name, build_dir_name
        return 0

    def resolve_output_log_path(
        self,
        *,
        app_name: str,
        tidy: bool,
        build_dir_name: str | None = None,
        profile_name: str | None = None,
    ) -> Path:
        _ = app_name, tidy, build_dir_name, profile_name
        return self.ctx.repo_root / "out" / "fake" / "build.log"


class TestBuildCommandEntries(TestCase):
    def test_build_entry_prints_cmake_build_dir_after_success(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_text(
                repo_root / "tools" / "toolchain" / "config.toml",
                """
[apps.demo]
path = "apps/demo"
""".strip(),
            )
            ctx = Context(repo_root)
            command = _FakeBuildCommand(ctx, backend="cmake")
            captured = io.StringIO()

            with patch(
                "tools.toolchain.commands.cmd_build.command_entries.build_cmake.build_cmake",
                return_value=0,
            ), redirect_stdout(captured):
                ret = command_entries.build_entry(
                    command=command,
                    app_name="demo",
                    tidy=False,
                    build_dir_name="build_demo",
                    run_command_fn=lambda *args, **kwargs: 0,
                    kill_build_processes_fn=lambda: None,
                    kill_runtime_lock_processes_fn=lambda: None,
                )

            self.assertEqual(ret, 0)
            self.assertIn(
                "Build files have been written to: "
                + ctx.get_build_dir("demo", "build_demo").as_posix(),
                captured.getvalue(),
            )

    def test_build_entry_prints_cargo_build_dir_after_success(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_text(
                repo_root / "tools" / "toolchain" / "config.toml",
                """
[apps.tracer_windows_rust_cli]
path = "apps/cli/windows/rust"
backend = "cargo"
""".strip(),
            )
            ctx = Context(repo_root)
            command = _FakeBuildCommand(ctx, backend="cargo")
            captured = io.StringIO()

            with patch(
                "tools.toolchain.commands.cmd_build.command_entries.build_cargo.build_cargo",
                return_value=0,
            ), redirect_stdout(captured):
                ret = command_entries.build_entry(
                    command=command,
                    app_name="tracer_windows_rust_cli",
                    tidy=False,
                    build_dir_name="build",
                    profile_name=None,
                    run_command_fn=lambda *args, **kwargs: 0,
                    kill_build_processes_fn=lambda: None,
                    kill_runtime_lock_processes_fn=lambda: None,
                )

            self.assertEqual(ret, 0)
            self.assertIn(
                "Build files have been written to: "
                + ctx.get_build_dir("tracer_windows_rust_cli", "build").as_posix(),
                captured.getvalue(),
            )
