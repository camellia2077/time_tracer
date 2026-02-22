import argparse

from ...commands.tidy.close import TidyCloseCommand
from ...core.context import Context
from ..common import add_profile_arg
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument(
        "--build-dir",
        default=None,
        help=(
            "Build directory for final verify stage only "
            "(default: build_fast or profile build_dir)."
        ),
    )
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Use concise output for verify test runner.",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before verify build stages (default: off)",
    )
    parser.add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    tidy_close_keep_going_group = parser.add_mutually_exclusive_group()
    tidy_close_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Continue incremental chunks after a non-zero clang-tidy exit.",
    )
    tidy_close_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Stop incremental refresh on first non-zero clang-tidy exit.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    cmd = TidyCloseCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        keep_going=args.keep_going,
        verify_build_dir_name=args.build_dir,
        profile_name=args.profile,
        concise=args.concise,
        kill_build_procs=kill_build_procs,
    )


COMMAND = CommandSpec(name="tidy-close", register=register, run=run)
