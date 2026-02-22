import sys
from pathlib import Path

from ...core.context import Context
from ...core.executor import run_command


class SelfTestCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(self, pattern: str = "test_*.py", verbose: bool = True) -> int:
        tests_dir = self.ctx.repo_root / "scripts" / "tests"
        if not tests_dir.exists():
            print(f"Error: scripts tests directory not found: {tests_dir}")
            return 1

        cmd = [
            sys.executable,
            "-m",
            "unittest",
            "discover",
            "-s",
            str(Path("scripts/tests")),
            "-p",
            pattern,
        ]
        if verbose:
            cmd.append("-v")
        return run_command(cmd, cwd=self.ctx.repo_root, env=self.ctx.setup_env())
