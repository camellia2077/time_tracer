import argparse

from ...commands.cmd_build import BuildCommand
from ...core.context import Context
from ..common import add_profile_arg, parse_cmake_args
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument("--tidy", action="store_true")
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override build directory name (default: build_fast/build_tidy).",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before configure/build (default: off)",
    )
    parser.add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )
    parser.add_argument(
        "--cmake-args",
        action="append",
        default=[],
        metavar="ARGS",
        help=(
            "Extra CMake configure args string. "
            "Can be repeated. Recommended: --cmake-args=-DENABLE_LTO=OFF"
        ),
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    cmd = BuildCommand(ctx)
    return cmd.configure(
        args.app,
        args.tidy,
        args.extra_args,
        cmake_args=parse_cmake_args(getattr(args, "cmake_args", [])),
        build_dir_name=args.build_dir,
        profile_name=args.profile,
        kill_build_procs=kill_build_procs,
    )


COMMAND = CommandSpec(name="configure", register=register, run=run)
