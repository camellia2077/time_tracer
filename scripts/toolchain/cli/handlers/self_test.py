import argparse

from ...commands.cmd_quality.self_test import SelfTestCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument(
        "--pattern",
        default="test_*.py",
        help="unittest discovery pattern (default: test_*.py).",
    )
    parser.add_argument(
        "--quiet",
        action="store_true",
        help="Disable verbose unittest output.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = SelfTestCommand(ctx)
    return cmd.execute(pattern=args.pattern, verbose=not args.quiet)


COMMAND = CommandSpec(
    name="self-test",
    register=register,
    run=run,
    app_mode="none",
    add_app_path=False,
    help="Run scripts/toolchain minimal regression tests.",
)
