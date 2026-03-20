import argparse
import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[3]

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.cli.handlers import validate as validate_handler  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402


class TestValidateCliHandler(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ctx = Context(REPO_ROOT)

    def test_run_flattens_paths_and_passes_flags(self):
        args = argparse.Namespace(
            plan="temp/import_batch01.toml",
            paths=[["libs/a.cpp", "libs/b.cpp"]],
            paths_file="temp/import_batch01.paths",
            run_name="batch01",
            verbose=True,
        )

        with patch("tools.toolchain.cli.handlers.validate.ValidateCommand") as mocked_command:
            mocked_command.return_value.execute.return_value = 0
            result = validate_handler.run(args, self.ctx)

        self.assertEqual(result, 0)
        kwargs = mocked_command.return_value.execute.call_args.kwargs
        self.assertEqual(kwargs["plan_path"], "temp/import_batch01.toml")
        self.assertEqual(kwargs["raw_paths"], ["libs/a.cpp", "libs/b.cpp"])
        self.assertEqual(kwargs["paths_file"], "temp/import_batch01.paths")
        self.assertEqual(kwargs["run_name"], "batch01")
        self.assertTrue(kwargs["verbose"])

    def test_run_passes_quiet_mode(self):
        args = argparse.Namespace(
            plan="temp/import_batch01.toml",
            paths=[],
            paths_file=None,
            run_name=None,
            verbose=False,
        )

        with patch("tools.toolchain.cli.handlers.validate.ValidateCommand") as mocked_command:
            mocked_command.return_value.execute.return_value = 0
            result = validate_handler.run(args, self.ctx)

        self.assertEqual(result, 0)
        kwargs = mocked_command.return_value.execute.call_args.kwargs
        self.assertFalse(kwargs["verbose"])
