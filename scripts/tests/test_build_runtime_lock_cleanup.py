import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import Mock, patch

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

from toolchain.commands.cmd_build import command_entries  # noqa: E402


class _FakeCommand:
    def __init__(self, backend: str = "cmake", sync_ret: int = 0):
        self.ctx = object()
        self._backend = backend
        self._sync_ret = sync_ret

    def _sync_platform_config_if_needed(self, _app_name: str) -> int:
        return self._sync_ret

    def _resolve_backend(self, _app_name: str) -> str:
        return self._backend

    def resolve_build_dir_name(self, **_kwargs) -> str:
        return "build_fast"

    def _is_configured(self, **_kwargs) -> bool:
        return True

    def _needs_windows_config_reconfigure(self, *_args, **_kwargs) -> bool:
        return False

    def configure(self, *_args, **_kwargs) -> int:
        return 0

    def _sync_windows_runtime_config_copy_if_needed(self, *_args, **_kwargs) -> int:
        return 0


class TestBuildRuntimeLockCleanup(TestCase):
    def test_build_entry_cleans_runtime_lock_processes_for_windows_core_apps(self):
        command = _FakeCommand(backend="cmake")
        cleanup_runtime = Mock()
        cleanup_build = Mock()

        with patch(
            "toolchain.commands.cmd_build.command_entries.build_cmake.build_cmake",
            return_value=0,
        ):
            ret = command_entries.build_entry(
                command=command,
                app_name="tracer_windows_cli",
                tidy=False,
                extra_args=[],
                cmake_args=[],
                build_dir_name="build_fast",
                profile_name="fast",
                kill_build_procs=False,
                run_command_fn=Mock(return_value=0),
                kill_build_processes_fn=cleanup_build,
                kill_runtime_lock_processes_fn=cleanup_runtime,
            )

        self.assertEqual(ret, 0)
        cleanup_runtime.assert_called_once()
        cleanup_build.assert_not_called()

    def test_build_entry_skips_runtime_lock_cleanup_for_non_windows_core_apps(self):
        command = _FakeCommand(backend="gradle")
        cleanup_runtime = Mock()
        cleanup_build = Mock()

        with patch(
            "toolchain.commands.cmd_build.command_entries.build_gradle.build_gradle",
            return_value=0,
        ):
            ret = command_entries.build_entry(
                command=command,
                app_name="tracer_android",
                tidy=False,
                extra_args=[],
                cmake_args=[],
                build_dir_name="build",
                profile_name="fast",
                kill_build_procs=False,
                run_command_fn=Mock(return_value=0),
                kill_build_processes_fn=cleanup_build,
                kill_runtime_lock_processes_fn=cleanup_runtime,
            )

        self.assertEqual(ret, 0)
        cleanup_runtime.assert_not_called()
        cleanup_build.assert_not_called()
