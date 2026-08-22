from pathlib import Path
from unittest import TestCase

from tools.toolchain.commands.cmd_build.gradle import build_gradle
from tools.toolchain.core.context import Context


class TestBuildGradleGuardrails(TestCase):
    def test_tracer_android_adds_plain_console_guardrail(self) -> None:
        repo_root = Path(__file__).resolve().parents[4]
        ctx = Context(repo_root)
        captured: list[str] = []

        def fake_run_command(cmd, **kwargs):
            _ = kwargs
            captured.extend(str(part) for part in cmd)
            return 0

        result = build_gradle(
            ctx=ctx,
            app_name="tracer_android",
            tidy=False,
            extra_args=[],
            cmake_args=[],
            build_dir_name=None,
            profile_name="android_style",
            run_command_fn=fake_run_command,
            output_mode="quiet",
        )

        self.assertEqual(result, 0)
        self.assertIn("--console=plain", captured)

    def test_explicit_gradle_task_override_keeps_android_guardrails(self) -> None:
        repo_root = Path(__file__).resolve().parents[4]
        ctx = Context(repo_root)
        captured: list[str] = []

        def fake_run_command(cmd, **kwargs):
            _ = kwargs
            captured.extend(str(part) for part in cmd)
            return 0

        result = build_gradle(
            ctx=ctx,
            app_name="tracer_android",
            tidy=False,
            extra_args=["--tests", "example.Test"],
            cmake_args=[],
            build_dir_name=None,
            profile_name=None,
            gradle_tasks_override=[":feature-insights:testDebugUnitTest"],
            run_command_fn=fake_run_command,
            output_mode="quiet",
        )

        self.assertEqual(result, 0)
        self.assertIn(":feature-insights:testDebugUnitTest", captured)
        self.assertNotIn(":app:assembleDebug", captured)
        self.assertIn("--tests", captured)
        self.assertIn("--no-parallel", captured)
