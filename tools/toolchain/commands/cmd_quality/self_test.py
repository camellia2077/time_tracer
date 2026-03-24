import sys
from pathlib import Path

from ...core.context import Context
from ...core.executor import run_command
from ....tests.groups import TEST_GROUPS


class SelfTestCommand:
    def __init__(self, ctx: Context):
        self.ctx = ctx

    def execute(
        self,
        pattern: str = "test_*.py",
        verbose: bool = True,
        group: str | None = None,
        run_command_fn=None,
    ) -> int:
        tests_dir = self.ctx.repo_root / "tools" / "tests"
        if not tests_dir.exists():
            print(f"Error: tools tests directory not found: {tests_dir}")
            return 1

        if group:
            if group not in TEST_GROUPS:
                print(f"Error: unsupported self-test group `{group}`")
                return 2
            cmd = [
                sys.executable,
                "-m",
                "unittest",
                *TEST_GROUPS[group],
            ]
        else:
            cmd = [
                sys.executable,
                "-m",
                "unittest",
                "discover",
                "-s",
                str(Path("tools/tests")),
                "-p",
                pattern,
            ]
        if verbose:
            cmd.append("-v")
        effective_run_command = run_command if run_command_fn is None else run_command_fn
        return effective_run_command(cmd, cwd=self.ctx.repo_root, env=self.ctx.setup_env())
