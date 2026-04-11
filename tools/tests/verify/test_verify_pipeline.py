import io
from contextlib import redirect_stdout
from pathlib import Path
from unittest import TestCase

from tools.toolchain.commands.cmd_quality.verify_internal.verify_pipeline import (
    run_artifact_pipeline,
)


class TestVerifyPipeline(TestCase):
    def test_artifact_pipeline_runs_native_then_blackbox_then_quality_gates(self):
        calls: list[str] = []
        stdout = io.StringIO()

        def fake_run_command(cmd, cwd=None, env=None):
            _ = cwd, env
            calls.append("suite")
            self.assertEqual(cmd, ["python", "test_suite.py"])
            return 0

        def fake_markdown_gates(**kwargs):
            _ = kwargs
            calls.append("markdown")
            return 0

        def fake_native(**kwargs):
            _ = kwargs
            calls.append("native")
            return 0

        with redirect_stdout(stdout):
            result = run_artifact_pipeline(
                test_cmd=["python", "test_suite.py"],
                app_name="tracer_core_shell",
                build_dir_name="build_fast",
                profile_name="cap_query",
                repo_root=Path.cwd(),
                setup_env_fn=lambda: {},
                run_command_fn=fake_run_command,
                run_report_markdown_gates_fn=fake_markdown_gates,
                run_native_core_runtime_tests_fn=fake_native,
            )

        self.assertEqual(result, 0)
        self.assertEqual(calls, ["native", "suite", "markdown"])
        output = stdout.getvalue()
        self.assertIn("artifact phase [native_foundation]", output)
        self.assertIn("artifact phase [host_blackbox]", output)
        self.assertIn("artifact phase [golden_or_quality_gates]", output)

    def test_artifact_pipeline_stops_after_native_failure(self):
        calls: list[str] = []

        def fake_run_command(cmd, cwd=None, env=None):
            _ = cmd, cwd, env
            calls.append("suite")
            return 0

        def fake_markdown_gates(**kwargs):
            _ = kwargs
            calls.append("markdown")
            return 0

        def fake_native(**kwargs):
            _ = kwargs
            calls.append("native")
            return 9

        result = run_artifact_pipeline(
            test_cmd=["python", "test_suite.py"],
            app_name="tracer_core_shell",
            build_dir_name="build_fast",
            profile_name="cap_query",
            repo_root=Path.cwd(),
            setup_env_fn=lambda: {},
            run_command_fn=fake_run_command,
            run_report_markdown_gates_fn=fake_markdown_gates,
            run_native_core_runtime_tests_fn=fake_native,
        )

        self.assertEqual(result, 9)
        self.assertEqual(calls, ["native"])
