import tempfile
from pathlib import Path
from unittest import TestCase

from tools.toolchain.commands.cmd_validate.plan import load_validation_plan
from tools.toolchain.commands.cmd_validate.scope import resolve_scope_paths


class TestValidatePlan(TestCase):
    def test_load_validation_plan_applies_defaults_and_override(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            plan_path = repo_root / "temp" / "validate.toml"
            plan_path.parent.mkdir(parents=True, exist_ok=True)
            plan_path.write_text(
                "\n".join(
                    [
                        "[run]",
                        'name = "import batch 01"',
                        "continue_on_failure = false",
                        "",
                        "[defaults]",
                        'kind = "verify"',
                        'app = "tracer_core"',
                        'verify_scope = "batch"',
                        "concise = true",
                        "",
                        "[[tracks]]",
                        'name = "modules_on"',
                        'build_dir = "build_on"',
                        "",
                        "[[tracks]]",
                        'name = "modules_off"',
                        'build_dir = "build_off"',
                        'cmake_args = ["-DTT_ENABLE_CPP20_MODULES=OFF"]',
                    ]
                )
                + "\n",
                encoding="utf-8",
            )

            plan = load_validation_plan(plan_path, run_name_override="custom run")

        self.assertEqual(plan.run_name, "custom_run")
        self.assertFalse(plan.continue_on_failure)
        self.assertEqual(len(plan.tracks), 2)
        self.assertEqual(plan.tracks[0].app, "tracer_core")
        self.assertEqual(plan.tracks[0].verify_scope, "batch")
        self.assertEqual(plan.tracks[1].cmake_args, ["-DTT_ENABLE_CPP20_MODULES=OFF"])

    def test_resolve_scope_paths_merges_cli_and_file(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            (repo_root / "libs" / "tracer_core" / "src").mkdir(parents=True, exist_ok=True)
            paths_file = repo_root / "temp" / "scope.paths"
            paths_file.parent.mkdir(parents=True, exist_ok=True)
            paths_file.write_text(
                "\n".join(
                    [
                        "# comment",
                        "libs/tracer_core/src/b.cpp",
                        "libs/tracer_core/src/a.cpp",
                    ]
                )
                + "\n",
                encoding="utf-8",
            )

            scope_paths = resolve_scope_paths(
                repo_root,
                raw_paths=["libs/tracer_core/src/a.cpp"],
                paths_file=str(paths_file),
            )

        self.assertEqual(
            scope_paths,
            [
                "libs/tracer_core/src/a.cpp",
                "libs/tracer_core/src/b.cpp",
            ],
        )

    def test_resolve_scope_paths_requires_explicit_input(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            with self.assertRaisesRegex(ValueError, "requires --paths or --paths-file"):
                resolve_scope_paths(Path(temp_dir), raw_paths=[], paths_file=None)
