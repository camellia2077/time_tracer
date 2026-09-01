from pathlib import Path
from tempfile import TemporaryDirectory
from unittest import TestCase
from unittest.mock import patch

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

    def test_android_does_not_retry_unrelated_gradle_failure(self) -> None:
        repo_root = Path(__file__).resolve().parents[4]
        ctx = Context(repo_root)
        calls = 0

        def fake_run_command(cmd, **kwargs):
            nonlocal calls
            _ = cmd
            calls += 1
            kwargs["log_file"].write_text(
                "e: Unresolved reference: missingSymbol\n",
                encoding="utf-8",
            )
            return 1

        with TemporaryDirectory() as temp_dir:
            log_file = Path(temp_dir) / "gradle.log"
            result = build_gradle(
                ctx=ctx,
                app_name="tracer_android",
                tidy=False,
                extra_args=[],
                cmake_args=[],
                build_dir_name=None,
                profile_name=None,
                gradle_tasks_override=[":feature-insights:testDebugUnitTest"],
                run_command_fn=fake_run_command,
                log_file=log_file,
                output_mode="quiet",
            )

        self.assertEqual(result, 1)
        self.assertEqual(calls, 1)

    def test_android_retries_and_preserves_log_for_kotlinc_intermediate_failure(self) -> None:
        repo_root = Path(__file__).resolve().parents[4]
        ctx = Context(repo_root)
        calls = 0

        def fake_run_command(cmd, **kwargs):
            nonlocal calls
            _ = cmd
            calls += 1
            if calls == 1:
                kwargs["log_file"].write_text(
                    "built_in_kotlinc intermediate is locked\n",
                    encoding="utf-8",
                )
                return 1
            kwargs["log_file"].write_text("BUILD SUCCESSFUL\n", encoding="utf-8")
            return 0

        with TemporaryDirectory() as temp_dir:
            log_file = Path(temp_dir) / "gradle.log"
            with patch(
                "tools.toolchain.commands.cmd_build.gradle._clear_android_built_in_kotlinc_dirs",
                return_value=1,
            ):
                result = build_gradle(
                    ctx=ctx,
                    app_name="tracer_android",
                    tidy=False,
                    extra_args=[],
                    cmake_args=[],
                    build_dir_name=None,
                    profile_name=None,
                    gradle_tasks_override=[":feature-insights:testDebugUnitTest"],
                    run_command_fn=fake_run_command,
                    log_file=log_file,
                    output_mode="quiet",
                )

            self.assertTrue(log_file.with_name("gradle.attempt-1.log").is_file())

        self.assertEqual(result, 0)
        self.assertEqual(calls, 2)
