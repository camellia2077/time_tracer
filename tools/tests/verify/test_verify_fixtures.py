import sys
from pathlib import Path
from unittest import TestCase

REPO_ROOT = Path(__file__).resolve().parents[3]
TOOLS_DIR = REPO_ROOT / "tools"

if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.toolchain.commands.cmd_quality.verify import VerifyCommand  # noqa: E402
from tools.toolchain.core.context import Context  # noqa: E402


class VerifyCommandTestBase(TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ctx = Context(REPO_ROOT)
        cls.command = VerifyCommand(cls.ctx)


def make_fake_build_command(exit_code: int, build_dir_name: str = "build_fast"):
    class FakeBuildCommand:
        def __init__(self, _ctx):
            pass

        def build(self, **_kwargs):
            return exit_code

        def resolve_build_dir_name(self, **_kwargs):
            return build_dir_name

    return FakeBuildCommand
