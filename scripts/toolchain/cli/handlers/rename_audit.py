import argparse

from ...commands.cmd_rename import RenameCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument("--strict", action="store_true")


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = RenameCommand(ctx)
    return cmd.audit(args.app, strict=args.strict)


COMMAND = CommandSpec(name="rename-audit", register=register, run=run)
