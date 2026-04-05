import sys
import importlib.util
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[4]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

RUN_SUPPORT_PATH = REPO_ROOT / "test" / "framework" / "runner" / "commands" / "run_support.py"
RUN_SUPPORT_SPEC = importlib.util.spec_from_file_location("tt_run_support", RUN_SUPPORT_PATH)
run_support = importlib.util.module_from_spec(RUN_SUPPORT_SPEC)
assert RUN_SUPPORT_SPEC is not None
assert RUN_SUPPORT_SPEC.loader is not None
RUN_SUPPORT_SPEC.loader.exec_module(run_support)


class TestTestRunnerBuildSupport(TestCase):
    def test_resolve_runtime_platform_args_for_windows_rust_cli(self):
        self.assertEqual(
            run_support.resolve_runtime_platform_args("tracer_windows_rust_cli"),
            ["--runtime-platform", "windows"],
        )
        self.assertEqual(run_support.resolve_runtime_platform_args("tracer_android"), [])

    def test_run_optional_build_steps_adds_runtime_platform_to_windows_rust_cli_build(self):
        recorded_calls = []

        def _fake_run_step(title, cmd, cwd):
            recorded_calls.append((title, cmd, cwd))
            return 0

        with patch.object(run_support, "run_step", side_effect=_fake_run_step):
            ret = run_support.run_optional_build_steps(
                repo_root=REPO_ROOT,
                app_name="tracer_windows_rust_cli",
                scripts_run=REPO_ROOT / "tools" / "run.py",
                python_exe=sys.executable,
                effective_build_dir="build_fast",
                tidy=False,
                kill_build_procs=False,
                skip_configure=False,
                skip_build=False,
            )

        self.assertEqual(ret, 0)
        self.assertEqual(len(recorded_calls), 2)
        configure_cmd = recorded_calls[0][1]
        build_cmd = recorded_calls[1][1]
        self.assertNotIn("--runtime-platform", configure_cmd)
        self.assertIn("--runtime-platform", build_cmd)
        self.assertIn("windows", build_cmd)
