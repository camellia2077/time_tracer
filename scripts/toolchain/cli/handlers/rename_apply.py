import argparse

from ...commands.cmd_rename import RenameCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument("--limit", type=int, default=0)
    parser.add_argument("--dry-run", action="store_true")
    parser.add_argument("--strict", action="store_true")


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = RenameCommand(ctx)
    return cmd.apply(args.app, limit=args.limit, dry_run=args.dry_run, strict=args.strict)


COMMAND = CommandSpec(name="rename-apply", register=register, run=run)
