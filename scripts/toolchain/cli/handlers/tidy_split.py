import argparse

from ...commands.tidy import TidyCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument(
        "--parse-workers",
        type=int,
        default=None,
        help="Parallel workers for log splitting.",
    )
    parser.add_argument(
        "--build-dir",
        default=None,
        help="Override tidy build directory name (default: build_tidy).",
    )
    parser.add_argument(
        "--max-lines",
        type=int,
        default=None,
        help="Max lines per generated task log (default from config).",
    )
    parser.add_argument(
        "--max-diags",
        type=int,
        default=None,
        help="Max diagnostics per generated task log (default from config).",
    )
    parser.add_argument(
        "--batch-size",
        type=int,
        default=None,
        help="Max task logs per batch folder (default from config).",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyCommand(ctx)
    return cmd.split_only(
        app_name=args.app,
        parse_workers=args.parse_workers,
        max_lines=args.max_lines,
        max_diags=args.max_diags,
        batch_size=args.batch_size,
        build_dir_name=args.build_dir,
    )


COMMAND = CommandSpec(name="tidy-split", register=register, run=run)
