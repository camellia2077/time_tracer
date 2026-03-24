import argparse
import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[3]
TOOLS_DIR = REPO_ROOT / "tools"

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.cli.handlers.quality import verify as verify_handler  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402


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
            build_dir=None,
            kill_build_procs=False,
            no_kill_build_procs=False,
            cmake_args=[],
            concise=False,
            extra_args=[],
        )

    def test_run_with_concise_keeps_explicit_build_dir(self):
        args = self._base_args()
        args.concise = True
        args.build_dir = "custom_build"

        with patch("tools.toolchain.cli.handlers.quality.verify.VerifyCommand") as mocked_command:
            mocked_command.return_value.execute.return_value = 0
            result = verify_handler.run(args, self.ctx)

        self.assertEqual(result, 0)
        kwargs = mocked_command.return_value.execute.call_args.kwargs
        self.assertEqual(kwargs["build_dir_name"], "custom_build")
        self.assertTrue(kwargs["concise"])
