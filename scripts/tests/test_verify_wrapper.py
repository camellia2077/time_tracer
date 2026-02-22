import importlib.util
import sys
from pathlib import Path
from types import SimpleNamespace
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[2]
SCRIPTS_DIR = REPO_ROOT / "scripts"
VERIFY_PATH = SCRIPTS_DIR / "verify.py"


def _load_verify_module():
    spec = importlib.util.spec_from_file_location("scripts_verify_wrapper", VERIFY_PATH)
    if spec is None or spec.loader is None:
        raise RuntimeError("Failed to load scripts/verify.py module spec")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestVerifyWrapper(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.verify_module = _load_verify_module()

    def test_main_defaults_to_help(self):
        with patch.object(self.verify_module.subprocess, "run") as run_mock:
            run_mock.return_value = SimpleNamespace(returncode=0)
            ret = self.verify_module.main([])

        self.assertEqual(ret, 0)
        cmd = run_mock.call_args.args[0]
        self.assertEqual(cmd[0], sys.executable)
        self.assertTrue(
            str(cmd[1]).endswith("scripts\\run.py") or str(cmd[1]).endswith("scripts/run.py")
        )
        self.assertEqual(cmd[2], "verify")
        self.assertEqual(cmd[3:], ["--help"])

    def test_main_strips_leading_verify_token(self):
        with patch.object(self.verify_module.subprocess, "run") as run_mock:
            run_mock.return_value = SimpleNamespace(returncode=7)
            ret = self.verify_module.main(["verify", "--app", "time_tracer"])

        self.assertEqual(ret, 7)
        cmd = run_mock.call_args.args[0]
        self.assertEqual(cmd[2], "verify")
        self.assertEqual(cmd[3:], ["--app", "time_tracer"])
