import argparse

from ...commands.analyze import AnalyzeCommand
from ...core.context import Context
from ..common import add_source_scope_arg, reject_unsupported_build_dir_override
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used to resolve the default analyze workspace when --build-dir is omitted.",
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override analyze build directory name (default: build_analyze or build_analyze_<scope>).",
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=None,
        help="Max issue JSON artifacts per batch folder (default: 10).",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    build_dir_error = reject_unsupported_build_dir_override(
        ctx=ctx,
        app_name=args.app,
        build_dir_name=args.build_dir,
        command_name="analyze-split",
    )
    if build_dir_error != 0:
        return build_dir_error
    cmd = AnalyzeCommand(ctx)
    return cmd.split_only(
        args.app,
        source_scope=args.source_scope,
        build_dir_name=args.build_dir,
        batch_size=args.batch_size,
    )


COMMAND = CommandSpec(name="analyze-split", register=register, run=run)
