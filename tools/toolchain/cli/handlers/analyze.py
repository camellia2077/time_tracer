import argparse

from ...commands.analyze import AnalyzeCommand
from ...core.context import Context
from ..common import add_profile_arg, add_source_scope_arg, reject_unsupported_build_dir_override
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    parser.add_argument("--jobs", type=int, default=None, help="Ninja parallel jobs for prebuild targets.")
    add_profile_arg(parser, defaults)
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="When omitted, analyze all compile units in the selected workspace.",
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override analyze build directory name (default: build_analyze or build_analyze_<scope>).",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    build_dir_error = reject_unsupported_build_dir_override(
        ctx=ctx,
        app_name=args.app,
        build_dir_name=args.build_dir,
        command_name="analyze",
    )
    if build_dir_error != 0:
        return build_dir_error
    cmd = AnalyzeCommand(ctx)
    return cmd.execute(
        args.app,
        jobs=args.jobs,
        source_scope=args.source_scope,
        build_dir_name=args.build_dir,
        profile_name=args.profile,
    )


COMMAND = CommandSpec(name="analyze", register=register, run=run)
