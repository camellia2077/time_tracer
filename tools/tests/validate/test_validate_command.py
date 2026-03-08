import json
import tempfile
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

    def _write_plan(self, repo_root: Path, content: str) -> Path:
        plan_path = repo_root / "temp" / "validate.toml"
        plan_path.parent.mkdir(parents=True, exist_ok=True)
        plan_path.write_text(content, encoding="utf-8")
        return plan_path

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
                result = command.execute(
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
                result = command.execute(
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
                result = command.execute(
                    plan_path=str(plan_path),
                    raw_paths=["libs/tracer_core/src/a.cpp"],
                )

            self.assertEqual(result, 9)
            self.assertEqual(calls, ["build_one"])
            summary_path = repo_root / "out" / "validate" / "import_batch03" / "summary.json"
            payload = json.loads(summary_path.read_text(encoding="utf-8"))
            self.assertFalse(payload["success"])
            self.assertEqual(len(payload["tracks"]), 1)
