import argparse

from ...commands.cmd_quality.format import FormatCommand
from ...core.context import Context
from ..common import add_profile_arg
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override build directory name for format target.",
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = FormatCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        extra_args=args.extra_args,
        build_dir_name=args.build_dir,
        profile_name=args.profile,
    )


COMMAND = CommandSpec(name="format", register=register, run=run)
