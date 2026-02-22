import argparse

from ...commands.cmd_rename import RenameCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument("--max-candidates", type=int, default=None)
    parser.add_argument("--run-tidy", action="store_true")


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = RenameCommand(ctx)
    return cmd.plan(args.app, max_candidates=args.max_candidates, run_tidy=args.run_tidy)


COMMAND = CommandSpec(name="rename-plan", register=register, run=run)
