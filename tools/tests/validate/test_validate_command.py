import io
import json
import tempfile
from contextlib import redirect_stderr, redirect_stdout
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

from tools.toolchain.commands.cmd_validate import ValidateCommand
from tools.toolchain.core.context import Context


class TestValidateCommand(TestCase):
    def _write_minimal_repo(self, repo_root: Path) -> None:
        config_dir = repo_root / "tools" / "toolchain" / "config"
        config_dir.mkdir(parents=True, exist_ok=True)
        (config_dir / "apps.toml").write_text(
            "\n".join(
                [
                    "[apps.tracer_core]",
                    'path = "apps/tracer_core_shell"',
                    "",
                    "[apps.tracer_core_shell]",
                    'path = "apps/tracer_core_shell"',
                    "",
                ]
            )
            + "\n",
            encoding="utf-8",
        )

    def _write_demo_repo(self, repo_root: Path) -> None:
        config_dir = repo_root / "tools" / "toolchain" / "config"
        config_dir.mkdir(parents=True, exist_ok=True)
        (config_dir / "apps.toml").write_text(
            "\n".join(
                [
                    "[apps.demo]",
                    'path = "apps/demo"',
                    "",
                ]
            )
            + "\n",
            encoding="utf-8",
        )
        (repo_root / "apps" / "demo").mkdir(parents=True, exist_ok=True)

    def _write_plan(self, repo_root: Path, content: str) -> Path:
        plan_path = repo_root / "temp" / "validate.toml"
        plan_path.parent.mkdir(parents=True, exist_ok=True)
        plan_path.write_text(content, encoding="utf-8")
        return plan_path

    def _execute_silently(self, command: ValidateCommand, **kwargs) -> int:
        with redirect_stdout(io.StringIO()), redirect_stderr(io.StringIO()):
            return command.execute(**kwargs)

    def test_execute_writes_summary_for_successful_tracks(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_minimal_repo(repo_root)
            ctx = Context(repo_root)
            command = ValidateCommand(ctx)
            plan_path = self._write_plan(
                repo_root,
                "\n".join(
                    [
                        "[run]",
                        'name = "import_batch01"',
                        "",
                        "[defaults]",
                        'kind = "build"',
                        'app = "tracer_core"',
                        "",
                        "[[tracks]]",
                        'name = "modules_on"',
                        'build_dir = "build_on"',
                        "",
                        "[[tracks]]",
                        'name = "modules_off"',
                        'build_dir = "build_off"',
                    ]
                )
                + "\n",
            )

            def fake_build(self, **kwargs):
                print(f"build ok: {kwargs['build_dir_name']}")
                return 0

            with patch(
                "tools.toolchain.commands.cmd_validate.command.BuildCommand.build",
                new=fake_build,
            ):
                result = self._execute_silently(
                    command,
                    plan_path=str(plan_path),
                    raw_paths=["libs/tracer_core/src/a.cpp"],
                    verbose=False,
                )

            self.assertEqual(result, 0)
            summary_path = repo_root / "out" / "validate" / "import_batch01" / "summary.json"
            payload = json.loads(summary_path.read_text(encoding="utf-8"))
            self.assertTrue(payload["success"])
            self.assertEqual(payload["scope_paths"], ["libs/tracer_core/src/a.cpp"])
            self.assertEqual(len(payload["tracks"]), 2)
            self.assertEqual(payload["tracks"][0]["status"], "completed")
            self.assertTrue((repo_root / "out" / "validate" / "import_batch01" / "logs" / "output.log").exists())
            self.assertTrue(
                (repo_root / "out" / "validate" / "import_batch01" / "logs" / "output.full.log").exists()
            )

    def test_execute_uses_plan_scope_when_cli_scope_missing(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_minimal_repo(repo_root)
            ctx = Context(repo_root)
            command = ValidateCommand(ctx)
            plan_path = self._write_plan(
                repo_root,
                "\n".join(
                    [
                        "[run]",
                        'name = "import_batch_scope_default"',
                        "",
                        "[scope]",
                        'paths = ["libs/tracer_core/src/application/query/tree"]',
                        "",
                        "[defaults]",
                        'kind = "build"',
                        'app = "tracer_core"',
                        "",
                        "[[tracks]]",
                        'name = "modules_on"',
                        'build_dir = "build_on"',
                    ]
                )
                + "\n",
            )

            def fake_build(self, **kwargs):
                print(f"build ok: {kwargs['build_dir_name']}")
                return 0

            with patch(
                "tools.toolchain.commands.cmd_validate.command.BuildCommand.build",
                new=fake_build,
            ):
                result = self._execute_silently(
                    command,
                    plan_path=str(plan_path),
                    verbose=False,
                )

            self.assertEqual(result, 0)
            summary_path = (
                repo_root
                / "out"
                / "validate"
                / "import_batch_scope_default"
                / "summary.json"
            )
            payload = json.loads(summary_path.read_text(encoding="utf-8"))
            self.assertEqual(
                payload["scope_paths"],
                ["libs/tracer_core/src/application/query/tree"],
            )

    def test_execute_continues_and_collects_failure_summary_by_default(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_minimal_repo(repo_root)
            ctx = Context(repo_root)
            command = ValidateCommand(ctx)
            plan_path = self._write_plan(
                repo_root,
                "\n".join(
                    [
                        "[run]",
                        'name = "import_batch02"',
                        "",
                        "[defaults]",
                        'kind = "build"',
                        'app = "tracer_core"',
                        "",
                        "[[tracks]]",
                        'name = "first"',
                        'build_dir = "build_one"',
                        "",
                        "[[tracks]]",
                        'name = "second"',
                        'build_dir = "build_two"',
                    ]
                )
                + "\n",
            )
            calls: list[str] = []

            def fake_build(self, **kwargs):
                calls.append(kwargs["build_dir_name"])
                if kwargs["build_dir_name"] == "build_one":
                    print("fatal: first track failed")
                    return 5
                print("build ok: second")
                return 0

            with patch(
                "tools.toolchain.commands.cmd_validate.command.BuildCommand.build",
                new=fake_build,
            ):
                result = self._execute_silently(
                    command,
                    plan_path=str(plan_path),
                    raw_paths=["libs/tracer_core/src/a.cpp"],
                )

            self.assertEqual(result, 5)
            self.assertEqual(calls, ["build_one", "build_two"])
            summary_path = repo_root / "out" / "validate" / "import_batch02" / "summary.json"
            payload = json.loads(summary_path.read_text(encoding="utf-8"))
            self.assertFalse(payload["success"])
            self.assertEqual(len(payload["tracks"]), 2)
            self.assertEqual(payload["failures"][0]["step"], "build")
            self.assertIn("fatal: first track failed", payload["failures"][0]["key_error_lines"])

    def test_execute_stops_after_first_failure_when_plan_requests_it(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_minimal_repo(repo_root)
            ctx = Context(repo_root)
            command = ValidateCommand(ctx)
            plan_path = self._write_plan(
                repo_root,
                "\n".join(
                    [
                        "[run]",
                        'name = "import_batch03"',
                        "continue_on_failure = false",
                        "",
                        "[defaults]",
                        'kind = "build"',
                        'app = "tracer_core"',
                        "",
                        "[[tracks]]",
                        'name = "first"',
                        'build_dir = "build_one"',
                        "",
                        "[[tracks]]",
                        'name = "second"',
                        'build_dir = "build_two"',
                    ]
                )
                + "\n",
            )
            calls: list[str] = []

            def fake_build(self, **kwargs):
                calls.append(kwargs["build_dir_name"])
                print("fatal: stop here")
                return 9

            with patch(
                "tools.toolchain.commands.cmd_validate.command.BuildCommand.build",
                new=fake_build,
            ):
                result = self._execute_silently(
                    command,
                    plan_path=str(plan_path),
                    raw_paths=["libs/tracer_core/src/a.cpp"],
                )

            self.assertEqual(result, 9)
            self.assertEqual(calls, ["build_one"])
            summary_path = repo_root / "out" / "validate" / "import_batch03" / "summary.json"
            payload = json.loads(summary_path.read_text(encoding="utf-8"))
            self.assertFalse(payload["success"])
            self.assertEqual(len(payload["tracks"]), 1)

    def test_execute_verify_track_passes_concise_to_build_stage(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_minimal_repo(repo_root)
            ctx = Context(repo_root)
            command = ValidateCommand(ctx)
            plan_path = self._write_plan(
                repo_root,
                "\n".join(
                    [
                        "[run]",
                        'name = "import_batch04"',
                        "",
                        "[defaults]",
                        'kind = "verify"',
                        'app = "tracer_core"',
                        "concise = false",
                        "",
                        "[[tracks]]",
                        'name = "verify_track"',
                    ]
                )
                + "\n",
            )
            captured: list[bool] = []

            def fake_execute_build_stage(**kwargs):
                captured.append(bool(kwargs["concise"]))
                return 0, "build_test", "tracer_core", repo_root / "build.log"

            with (
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.execute_build_stage",
                    new=fake_execute_build_stage,
                ),
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.VerifyCommand.run_unit_scope_checks",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.run_suite_step",
                    return_value=None,
                ),
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.run_report_markdown_gates",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.run_native_core_runtime_tests",
                    return_value=0,
                ),
            ):
                result = self._execute_silently(
                    command,
                    plan_path=str(plan_path),
                    raw_paths=["libs/tracer_core/src/application/use_cases/query_api.cpp"],
                    verbose=False,
                )

            self.assertEqual(result, 0)
            self.assertEqual(captured, [False])

    def test_execute_verify_track_passes_profile_to_markdown_gates(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_minimal_repo(repo_root)
            ctx = Context(repo_root)
            command = ValidateCommand(ctx)
            plan_path = self._write_plan(
                repo_root,
                "\n".join(
                    [
                        "[run]",
                        'name = "import_batch04b"',
                        "",
                        "[defaults]",
                        'kind = "verify"',
                        'app = "tracer_core"',
                        'profile = "cap_query"',
                        "",
                        "[[tracks]]",
                        'name = "verify_track"',
                    ]
                )
                + "\n",
            )

            def fake_execute_build_stage(**_kwargs):
                return 0, "build_test", "tracer_core", repo_root / "build.log"

            with (
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.execute_build_stage",
                    new=fake_execute_build_stage,
                ),
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.VerifyCommand.run_unit_scope_checks",
                    return_value=0,
                ),
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.run_suite_step",
                    return_value=None,
                ),
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.run_report_markdown_gates",
                    return_value=0,
                ) as mocked_markdown,
                patch(
                    "tools.toolchain.commands.cmd_validate.verify_track.run_native_core_runtime_tests",
                    return_value=0,
                ),
            ):
                result = self._execute_silently(
                    command,
                    plan_path=str(plan_path),
                    raw_paths=["libs/tracer_core/src/application/use_cases/query_api.cpp"],
                    verbose=False,
                )

            self.assertEqual(result, 0)
            self.assertEqual(mocked_markdown.call_args.kwargs["profile_name"], "cap_query")

    def test_execute_build_and_configure_tracks_pass_concise_to_build_command(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_minimal_repo(repo_root)
            ctx = Context(repo_root)
            command = ValidateCommand(ctx)
            plan_path = self._write_plan(
                repo_root,
                "\n".join(
                    [
                        "[run]",
                        'name = "import_batch05"',
                        "",
                        "[defaults]",
                        'app = "tracer_core"',
                        "concise = true",
                        "",
                        "[[tracks]]",
                        'name = "configure_track"',
                        'kind = "configure"',
                        "",
                        "[[tracks]]",
                        'name = "build_track"',
                        'kind = "build"',
                    ]
                )
                + "\n",
            )
            configure_calls: list[bool] = []
            build_calls: list[bool] = []

            def fake_configure(self, **kwargs):
                configure_calls.append(bool(kwargs["concise"]))
                return 0

            def fake_build(self, **kwargs):
                build_calls.append(bool(kwargs["concise"]))
                return 0

            with (
                patch(
                    "tools.toolchain.commands.cmd_validate.command.BuildCommand.configure",
                    new=fake_configure,
                ),
                patch(
                    "tools.toolchain.commands.cmd_validate.command.BuildCommand.build",
                    new=fake_build,
                ),
            ):
                result = self._execute_silently(
                    command,
                    plan_path=str(plan_path),
                    raw_paths=["tools/toolchain/commands/cmd_validate/command.py"],
                    verbose=False,
                )

            self.assertEqual(result, 0)
            self.assertEqual(configure_calls, [True])
            self.assertEqual(build_calls, [True])

    def test_execute_configure_and_build_tracks_run_command_entry_smoke(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            self._write_demo_repo(repo_root)
            ctx = Context(repo_root)
            command = ValidateCommand(ctx)
            plan_path = self._write_plan(
                repo_root,
                "\n".join(
                    [
                        "[run]",
                        'name = "import_batch06"',
                        "",
                        "[defaults]",
                        'app = "demo"',
                        'build_dir = "build_smoke"',
                        "concise = true",
                        "",
                        "[[tracks]]",
                        'name = "configure_track"',
                        'kind = "configure"',
                        "",
                        "[[tracks]]",
                        'name = "build_track"',
                        'kind = "build"',
                    ]
                )
                + "\n",
            )
            captured_calls: list[tuple[str, str, Path]] = []

            def fake_configure_cmake(**kwargs):
                captured_calls.append(
                    (
                        "configure",
                        str(kwargs["output_mode"]),
                        Path(kwargs["log_file"]),
                    )
                )
                return 0

            def fake_build_cmake(**kwargs):
                captured_calls.append(
                    (
                        "build",
                        str(kwargs["output_mode"]),
                        Path(kwargs["log_file"]),
                    )
                )
                return 0

            with (
                patch(
                    "tools.toolchain.commands.cmd_build.command_entries.build_cmake.configure_cmake",
                    new=fake_configure_cmake,
                ),
                patch(
                    "tools.toolchain.commands.cmd_build.command_entries.build_cmake.build_cmake",
                    new=fake_build_cmake,
                ),
            ):
                result = self._execute_silently(
                    command,
                    plan_path=str(plan_path),
                    raw_paths=["tools/toolchain/commands/cmd_validate/command.py"],
                    verbose=False,
                )

            self.assertEqual(result, 0)
            self.assertEqual(
                [(kind, output_mode) for kind, output_mode, _ in captured_calls],
                [("configure", "quiet"), ("build", "quiet")],
            )
            self.assertTrue(all(log_file.name == "build.log" for _, _, log_file in captured_calls))
            summary_path = repo_root / "out" / "validate" / "import_batch06" / "summary.json"
            payload = json.loads(summary_path.read_text(encoding="utf-8"))
            self.assertTrue(payload["success"])
            self.assertEqual([track["kind"] for track in payload["tracks"]], ["configure", "build"])
            self.assertEqual(
                [track["status"] for track in payload["tracks"]],
                ["completed", "completed"],
            )
