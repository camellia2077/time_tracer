import argparse

from ...commands.cmd_build import BuildCommand
from ...core.context import Context
from ..common import add_profile_arg, parse_cmake_args, reject_unsupported_build_dir_override
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument("--tidy", action="store_true")
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--build-dir",
        default=None,
        help=(
            "Override build directory name for backends without a fixed build directory "
            "(for example CMake). Fixed-dir backends like `tracer_android` reject this flag."
        ),
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
            "Extra CMake configure args string applied before build. "
            "Can be repeated. Recommended: --cmake-args=-DENABLE_LTO=OFF"
        ),
    )
    parser.add_argument(
        "--windows-icon-svg",
        default=None,
        help=(
            "Override Windows CLI icon SVG source. "
            "Only used for tracer_windows_rust_cli release-profile builds."
        ),
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    build_dir_error = reject_unsupported_build_dir_override(
        ctx=ctx,
        app_name=args.app,
        build_dir_name=args.build_dir,
        command_name="build",
    )
    if build_dir_error != 0:
        return build_dir_error
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    cmd = BuildCommand(ctx)
    return cmd.build(
        app_name=args.app,
        tidy=args.tidy,
        extra_args=args.extra_args,
        cmake_args=parse_cmake_args(getattr(args, "cmake_args", [])),
        build_dir_name=args.build_dir,
        profile_name=args.profile,
        windows_icon_svg=getattr(args, "windows_icon_svg", None),
        kill_build_procs=kill_build_procs,
    )


COMMAND = CommandSpec(name="build", register=register, run=run)
