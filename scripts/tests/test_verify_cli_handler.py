import argparse
import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS_DIR = REPO_ROOT / "scripts"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

from toolchain.cli.handlers import verify as verify_handler  # noqa: E402
from toolchain.core.context import Context  # noqa: E402


class TestVerifyCliHandler(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ctx = Context(REPO_ROOT)

    @staticmethod
    def _base_args() -> argparse.Namespace:
        return argparse.Namespace(
            app="tracer_core",
            tidy=False,
            profile=None,
            quick=False,
            build_dir=None,
            kill_build_procs=False,
            no_kill_build_procs=False,
            cmake_args=[],
            concise=False,
            scope="batch",
            extra_args=[],
        )

    def test_run_quick_sets_build_fast_and_concise(self):
        args = self._base_args()
        args.quick = True

        with patch("toolchain.cli.handlers.verify.VerifyCommand") as mocked_command:
            mocked_command.return_value.execute.return_value = 0
            result = verify_handler.run(args, self.ctx)

        self.assertEqual(result, 0)
        kwargs = mocked_command.return_value.execute.call_args.kwargs
        self.assertEqual(kwargs["build_dir_name"], "build_fast")
        self.assertTrue(kwargs["concise"])

    def test_run_quick_keeps_explicit_build_dir(self):
        args = self._base_args()
        args.quick = True
        args.build_dir = "custom_build"

        with patch("toolchain.cli.handlers.verify.VerifyCommand") as mocked_command:
            mocked_command.return_value.execute.return_value = 0
            result = verify_handler.run(args, self.ctx)

        self.assertEqual(result, 0)
        kwargs = mocked_command.return_value.execute.call_args.kwargs
        self.assertEqual(kwargs["build_dir_name"], "custom_build")
        self.assertTrue(kwargs["concise"])

    def test_run_without_quick_keeps_original_flags(self):
        args = self._base_args()
        args.concise = False

        with patch("toolchain.cli.handlers.verify.VerifyCommand") as mocked_command:
            mocked_command.return_value.execute.return_value = 0
            result = verify_handler.run(args, self.ctx)

        self.assertEqual(result, 0)
        kwargs = mocked_command.return_value.execute.call_args.kwargs
        self.assertIsNone(kwargs["build_dir_name"])
        self.assertFalse(kwargs["concise"])
