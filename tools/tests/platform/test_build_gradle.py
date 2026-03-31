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

from tools.toolchain.commands.cmd_build import gradle as build_gradle  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402
from tools.toolchain.core.process_lock import ProcessLockBusyError  # noqa: E402


def _write_text(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(content, encoding="utf-8")


def _write_android_config(repo_root: Path) -> None:
    _write_text(
        repo_root / "tools" / "toolchain" / "config" / "test.toml",
        """
[apps.tracer_android]
path = "apps/android"
backend = "gradle"
fixed_build_dir = "build"
gradle_wrapper = "gradlew.bat"
gradle_platform = "android"
gradle_tasks = [
    ":app:assembleDebug",
]

[build.profiles.android_edit]
description = "Android edit-loop build only."
gradle_tasks = [
    ":app:assembleDebug",
]
""".strip(),
    )


class TestBuildGradle(TestCase):
    def test_build_gradle_runs_command_when_lock_is_available(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_android_config(repo_root)
            ctx = Context(repo_root)
            _write_text(ctx.get_app_dir("tracer_android") / "gradlew.bat", "@echo off\n")
            captured_calls: list[tuple[list[str], Path | None]] = []

            def _fake_run_command(cmd, cwd=None, env=None, log_file=None, output_mode="live"):
                _ = env, log_file, output_mode
                captured_calls.append((cmd, cwd))
                return 0

            ret = build_gradle.build_gradle(
                ctx=ctx,
                app_name="tracer_android",
                tidy=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name=None,
                profile_name="android_edit",
                run_command_fn=_fake_run_command,
            )

            self.assertEqual(ret, 0)
            self.assertEqual(len(captured_calls), 1)
            self.assertTrue(str(captured_calls[0][0][0]).endswith("gradlew.bat"))
            self.assertEqual(captured_calls[0][1], ctx.get_app_dir("tracer_android"))

    def test_build_gradle_rejects_parallel_run_when_lock_is_busy(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_android_config(repo_root)
            ctx = Context(repo_root)
            stdout = io.StringIO()

            with patch(
                "tools.toolchain.commands.cmd_build.gradle.hold_process_lock",
                side_effect=ProcessLockBusyError(
                    lock_path=ctx.get_out_root() / "locks" / "tracer_android" / "android_gradle.lock",
                    label="Android Gradle command for `tracer_android`",
                    owner_description="pid=4321, command=gradlew.bat :app:assembleDebug",
                ),
            ), redirect_stdout(stdout):
                ret = build_gradle.build_gradle(
                    ctx=ctx,
                    app_name="tracer_android",
                    tidy=False,
                    extra_args=None,
                    cmake_args=None,
                    build_dir_name=None,
                    profile_name="android_edit",
                    run_command_fn=lambda *args, **kwargs: 0,
                )

            self.assertEqual(ret, 1)
            output = stdout.getvalue()
            self.assertIn("must be serialized", output)
            self.assertIn("pid=4321", output)

    def test_build_gradle_merges_multiple_profiles_into_one_command(self):
        with TemporaryDirectory() as tmp:
            repo_root = Path(tmp)
            _write_android_config(repo_root)
            _write_text(
                repo_root / "tools" / "toolchain" / "config" / "profiles.toml",
                """
[build.profiles.android_style]
description = "Android style checks."
gradle_tasks = [
    ":app:ktlintCheck",
    ":app:lintDebug",
]

[build.profiles.android_ci]
description = "Android CI checks."
gradle_tasks = [
    ":runtime:verifyTracerCoreConfigSnapshot",
    ":app:check",
]
gradle_args = [
    "--stacktrace",
]
""".strip(),
            )
            ctx = Context(repo_root)
            _write_text(ctx.get_app_dir("tracer_android") / "gradlew.bat", "@echo off\n")
            captured_calls: list[list[str]] = []

            def _fake_run_command(cmd, cwd=None, env=None, log_file=None, output_mode="live"):
                _ = cwd, env, log_file, output_mode
                captured_calls.append(cmd)
                return 0

            ret = build_gradle.build_gradle(
                ctx=ctx,
                app_name="tracer_android",
                tidy=False,
                extra_args=None,
                cmake_args=None,
                build_dir_name=None,
                profile_name=["android_style", "android_ci"],
                run_command_fn=_fake_run_command,
            )

            self.assertEqual(ret, 0)
            self.assertEqual(len(captured_calls), 1)
            self.assertEqual(
                captured_calls[0][1:5],
                [
                    ":app:ktlintCheck",
                    ":app:lintDebug",
                    ":runtime:verifyTracerCoreConfigSnapshot",
                    ":app:check",
                ],
            )
            self.assertIn("--stacktrace", captured_calls[0])
