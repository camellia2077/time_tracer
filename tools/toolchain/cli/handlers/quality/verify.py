import argparse

from ....commands.cmd_quality.verify import VerifyCommand
from ....core.context import Context
from ...common import (
    add_profile_arg,
    parse_cmake_args,
    reject_unsupported_build_dir_override,
    resolve_fixed_build_dir,
)
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument("--tidy", action="store_true")
    add_profile_arg(parser, defaults)
    parser.add_argument(
        "--quick",
        action="store_true",
        help=(
            "Shortcut for concise verify output; also uses `build_fast` when the app "
            "does not declare a fixed backend build directory."
        ),
    )
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
        "--concise",
        action="store_true",
        help="Use concise output for test runner.",
    )
    parser.add_argument(
        "--scope",
        choices=["task", "unit", "artifact", "batch"],
        default="batch",
        help=(
            "Verification scope. "
            "`task` = lightweight task-level checks (build + native runtime smoke); "
            "`unit` = internal logic tests (Python unit/component); "
            "`artifact` = report/runtime artifact checks (suite + gates + native runtime); "
            "`batch` = unit + artifact full verify."
        ),
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    build_dir_name = args.build_dir
    concise = bool(args.concise)
    if args.quick:
        if build_dir_name is None and not resolve_fixed_build_dir(ctx, args.app):
            build_dir_name = "build_fast"
        concise = True
    build_dir_error = reject_unsupported_build_dir_override(
        ctx=ctx,
        app_name=args.app,
        build_dir_name=args.build_dir,
        command_name="verify",
    )
    if build_dir_error != 0:
        return build_dir_error
    cmd = VerifyCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        tidy=args.tidy,
        extra_args=args.extra_args,
        cmake_args=parse_cmake_args(getattr(args, "cmake_args", [])),
        build_dir_name=build_dir_name,
        profile_name=args.profile,
        concise=concise,
        kill_build_procs=kill_build_procs,
        verify_scope=args.scope,
    )


COMMAND = CommandSpec(name="verify", register=register, run=run)
