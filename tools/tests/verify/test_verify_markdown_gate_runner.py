from pathlib import Path
from unittest import TestCase
from unittest.mock import Mock

from tools.toolchain.commands.cmd_quality.verify_internal.verify_markdown_gate_runner import (
    run_report_markdown_gates,
)


REPO_ROOT = Path(__file__).resolve().parents[3]


class TestVerifyMarkdownGateRunner(TestCase):
    def test_cap_query_skips_reporting_markdown_gates(self):
        mocked_run = Mock(return_value=0)

        result = run_report_markdown_gates(
            repo_root=REPO_ROOT,
            setup_env_fn=lambda: {},
            run_command_fn=mocked_run,
            app_name="tracer_core_shell",
            build_dir_name="build_fast",
            profile_name="cap_query",
        )

        self.assertEqual(result, 0)
        mocked_run.assert_not_called()

    def test_cap_reporting_runs_reporting_markdown_gates(self):
        mocked_run = Mock(return_value=0)

        result = run_report_markdown_gates(
            repo_root=REPO_ROOT,
            setup_env_fn=lambda: {},
            run_command_fn=mocked_run,
            app_name="tracer_core_shell",
            build_dir_name="build_fast",
            profile_name="cap_reporting",
        )

        self.assertEqual(result, 0)
        called_commands = [" ".join(call.args[0]) for call in mocked_run.call_args_list]
        self.assertTrue(
            any("collect_report_markdown_cases.py" in command for command in called_commands)
        )

    def test_shell_aggregate_skips_reporting_markdown_gates(self):
        mocked_run = Mock(return_value=0)

        result = run_report_markdown_gates(
            repo_root=REPO_ROOT,
            setup_env_fn=lambda: {},
            run_command_fn=mocked_run,
            app_name="tracer_core_shell",
            build_dir_name="build_fast",
            profile_name="shell_aggregate",
        )

        self.assertEqual(result, 0)
        mocked_run.assert_not_called()
