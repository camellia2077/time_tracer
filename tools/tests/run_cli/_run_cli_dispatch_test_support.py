import importlib.util
import sys
from pathlib import Path
from unittest import TestCase
from unittest.mock import patch

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.cli.dispatch import dispatch_command  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402


def load_run_module():
    run_py = REPO_ROOT / "tools" / "run.py"
    spec = importlib.util.spec_from_file_location("tools_run_module", run_py)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"Failed to load module spec from {run_py}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class RunCliDispatchTestBase(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.run_module = load_run_module()

    def _assert_return_zero(self, argv: list[str]) -> None:
        with patch.object(sys, "argv", argv):
            rc = self.run_module.main()
        self.assertEqual(rc, 0)
