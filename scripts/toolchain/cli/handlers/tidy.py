import argparse

from ...commands.tidy import TidyCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument("--jobs", type=int, default=None, help="Ninja parallel jobs, e.g. 16")
    parser.add_argument(
        "--parse-workers",
        type=int,
        default=None,
        help="Parallel workers for log splitting",
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override tidy build directory name (default: build_tidy).",
    )
    tidy_keep_going_group = parser.add_mutually_exclusive_group()
    tidy_keep_going_group.add_argument(
        "--keep-going",
        dest="keep_going",
        action="store_true",
        default=None,
        help="Enable Ninja keep-going mode (-k 0) for tidy build.",
    )
    tidy_keep_going_group.add_argument(
        "--no-keep-going",
        dest="keep_going",
        action="store_false",
        help="Disable Ninja keep-going mode (-k 0) for tidy build.",
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyCommand(ctx)
    return cmd.execute(
        args.app,
        args.extra_args,
        jobs=args.jobs,
        parse_workers=args.parse_workers,
        keep_going=args.keep_going,
        build_dir_name=args.build_dir,
    )


COMMAND = CommandSpec(name="tidy", register=register, run=run)
