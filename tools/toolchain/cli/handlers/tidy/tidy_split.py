import argparse

from ....commands.tidy import TidyCommand
from ....core.context import Context
from ...common import add_source_scope_arg, add_tidy_task_view_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used to resolve the default scoped tidy workspace when --build-dir is omitted.",
    )
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
    add_tidy_task_view_arg(
        parser,
        choices=("text", "text+toon"),
        help_text=(
            "Optional human-facing task view(s) for tidy-split. Canonical task_*.json "
            "is always written for machine consumers; explicit `--task-view` here only "
            "supports `text` or `text+toon`."
        ),
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyCommand(ctx)
    return cmd.split_only(
        app_name=args.app,
        parse_workers=args.parse_workers,
        max_lines=args.max_lines,
        max_diags=args.max_diags,
        batch_size=args.batch_size,
        source_scope=args.source_scope,
        build_dir_name=args.build_dir,
        task_view=args.task_view,
    )


COMMAND = CommandSpec(name="tidy-split", register=register, run=run)
