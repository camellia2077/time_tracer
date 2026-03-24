import argparse

from ....commands.cmd_quality.verify import VerifyCommand
from ....core.context import Context
from ...common import (
    add_build_dir_arg,
    add_concise_arg,
    add_kill_build_procs_args,
    add_profile_arg,
    parse_cmake_args,
    reject_unsupported_build_dir_override,
)
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument("--tidy", action="store_true")
    add_profile_arg(parser, defaults)
    add_build_dir_arg(parser)
    add_kill_build_procs_args(parser)
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
    add_concise_arg(
        parser,
        help_text="Use concise top-level output and concise test runner output.",
    )
    parser.add_argument("extra_args", nargs=argparse.REMAINDER)


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
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
        build_dir_name=args.build_dir,
        profile_name=args.profile,
        concise=bool(args.concise),
        kill_build_procs=kill_build_procs,
    )


COMMAND = CommandSpec(name="verify", register=register, run=run)
