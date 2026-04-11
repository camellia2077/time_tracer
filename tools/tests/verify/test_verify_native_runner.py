from pathlib import Path
import tempfile
import io
from contextlib import redirect_stdout
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.cmd_quality.verify_internal.verify_native_runner import (
    run_native_core_runtime_tests,
)


class TestVerifyNativeRunner(TestCase):
    def test_cap_query_runs_query_native_targets(self):
        commands: list[list[str]] = []

        def fake_run_command(cmd, cwd=None, env=None):
            _ = cwd, env
            commands.append(list(cmd))
            return 0

        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            bin_dir = repo_root / "bin"
            bin_dir.mkdir(parents=True, exist_ok=True)
            for name in (
                "tt_query_api_tests.exe",
                "tc_c_api_query_tests.exe",
                "tc_app_query_mod_smoke_tests.exe",
                "tc_query_infra_smoke_tests.exe",
                "tt_android_runtime_query_tests.exe",
            ):
                (bin_dir / name).write_text("", encoding="utf-8")

            fake_layout = type("Layout", (), {"bin_dir": bin_dir})()
            with patch(
                "tools.toolchain.commands.cmd_quality.verify_internal.verify_native_runner.resolve_build_layout",
                return_value=fake_layout,
            ):
                result = run_native_core_runtime_tests(
                    repo_root=repo_root,
                    setup_env_fn=lambda: {},
                    run_command_fn=fake_run_command,
                    app_name="tracer_core_shell",
                    build_dir_name="build_fast",
                    profile_name="cap_query",
                )

        self.assertEqual(result, 0)
        executed = [Path(cmd[0]).name for cmd in commands]
        self.assertEqual(
            executed,
            [
                "tt_query_api_tests.exe",
                "tc_c_api_query_tests.exe",
                "tc_app_query_mod_smoke_tests.exe",
                "tc_query_infra_smoke_tests.exe",
                "tt_android_runtime_query_tests.exe",
            ],
        )

    def test_cap_query_reports_native_phases(self):
        commands: list[list[str]] = []

        def fake_run_command(cmd, cwd=None, env=None):
            _ = cwd, env
            commands.append(list(cmd))
            return 0

        stdout = io.StringIO()
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            bin_dir = repo_root / "bin"
            bin_dir.mkdir(parents=True, exist_ok=True)
            for name in (
                "tt_query_api_tests.exe",
                "tc_c_api_query_tests.exe",
                "tc_app_query_mod_smoke_tests.exe",
                "tc_query_infra_smoke_tests.exe",
                "tt_android_runtime_query_tests.exe",
            ):
                (bin_dir / name).write_text("", encoding="utf-8")

            fake_layout = type("Layout", (), {"bin_dir": bin_dir})()
            with (
                patch(
                    "tools.toolchain.commands.cmd_quality.verify_internal.verify_native_runner.resolve_build_layout",
                    return_value=fake_layout,
                ),
                redirect_stdout(stdout),
            ):
                result = run_native_core_runtime_tests(
                    repo_root=repo_root,
                    setup_env_fn=lambda: {},
                    run_command_fn=fake_run_command,
                    app_name="tracer_core_shell",
                    build_dir_name="build_fast",
                    profile_name="cap_query",
                )

        self.assertEqual(result, 0)
        output = stdout.getvalue()
        self.assertIn("native phase [core_semantics]", output)
        self.assertIn("native phase [c_abi_contract]", output)
        self.assertIn("native phase [runtime_smoke_and_bridge]", output)

    def test_cap_config_runs_config_native_targets(self):
        commands: list[list[str]] = []

        def fake_run_command(cmd, cwd=None, env=None):
            _ = cwd, env
            commands.append(list(cmd))
            return 0

        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            bin_dir = repo_root / "bin"
            bin_dir.mkdir(parents=True, exist_ok=True)
            for name in (
                "tc_config_infra_smoke_tests.exe",
                "tt_android_runtime_config_tests.exe",
            ):
                (bin_dir / name).write_text("", encoding="utf-8")

            fake_layout = type("Layout", (), {"bin_dir": bin_dir})()
            with patch(
                "tools.toolchain.commands.cmd_quality.verify_internal.verify_native_runner.resolve_build_layout",
                return_value=fake_layout,
            ):
                result = run_native_core_runtime_tests(
                    repo_root=repo_root,
                    setup_env_fn=lambda: {},
                    run_command_fn=fake_run_command,
                    app_name="tracer_core_shell",
                    build_dir_name="build_fast",
                    profile_name="cap_config",
                )

        self.assertEqual(result, 0)
        executed = [Path(cmd[0]).name for cmd in commands]
        self.assertEqual(
            executed,
            [
                "tc_config_infra_smoke_tests.exe",
                "tt_android_runtime_config_tests.exe",
            ],
        )

    def test_shell_aggregate_runs_shell_native_targets(self):
        commands: list[list[str]] = []

        def fake_run_command(cmd, cwd=None, env=None):
            _ = cwd, env
            commands.append(list(cmd))
            return 0

        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            bin_dir = repo_root / "bin"
            bin_dir.mkdir(parents=True, exist_ok=True)
            for name in (
                "tc_c_api_smoke_tests.exe",
                "tc_c_api_shell_aggregate_tests.exe",
                "ttr_tests.exe",
                "ttr_rt_codec_tests.exe",
                "tt_aggregate_runtime_tests.exe",
                "tc_app_aggregate_runtime_smoke_tests.exe",
                "tt_android_runtime_shell_smoke_tests.exe",
                "tt_file_crypto_runtime_bridge_tests.exe",
            ):
                (bin_dir / name).write_text("", encoding="utf-8")

            fake_layout = type("Layout", (), {"bin_dir": bin_dir})()
            with patch(
                "tools.toolchain.commands.cmd_quality.verify_internal.verify_native_runner.resolve_build_layout",
                return_value=fake_layout,
            ):
                result = run_native_core_runtime_tests(
                    repo_root=repo_root,
                    setup_env_fn=lambda: {},
                    run_command_fn=fake_run_command,
                    app_name="tracer_core_shell",
                    build_dir_name="build_fast",
                    profile_name="shell_aggregate",
                )

        self.assertEqual(result, 0)
        executed = [Path(cmd[0]).name for cmd in commands]
        self.assertEqual(
            executed,
            [
                "tc_c_api_smoke_tests.exe",
                "tc_c_api_shell_aggregate_tests.exe",
                "ttr_tests.exe",
                "ttr_rt_codec_tests.exe",
                "tt_aggregate_runtime_tests.exe",
                "tc_app_aggregate_runtime_smoke_tests.exe",
                "tt_android_runtime_shell_smoke_tests.exe",
                "tt_file_crypto_runtime_bridge_tests.exe",
            ],
        )
