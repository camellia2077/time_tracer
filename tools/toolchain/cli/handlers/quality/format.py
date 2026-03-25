import argparse

from ....commands.cmd_quality.format import FormatCommand
from ...common import print_cli_error
from ....core.context import Context
from ...common import add_profile_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override build directory name for format target.",
    )
    parser.add_argument(
        "--paths",
        nargs="+",
        default=None,
        help="Format or check explicit repository paths without going through an app build target.",
    )
    parser.add_argument(
        "--check",
        action="store_true",
        help="Check formatting without modifying files. For CMake apps this defaults to `check-format`.",
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    if not getattr(args, "app", None) and not getattr(args, "paths", None):
        print_cli_error("Error: `format` requires either `--app` or `--paths`.")
        return 2
    cmd = FormatCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        raw_paths=args.paths,
        check_only=args.check,
        extra_args=args.extra_args,
        build_dir_name=args.build_dir,
        profile_name=args.profile,
    )


COMMAND = CommandSpec(name="format", register=register, run=run, app_mode="optional")
