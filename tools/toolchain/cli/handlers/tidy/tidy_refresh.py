import argparse

from ....commands.tidy.execution.refresh import TidyRefreshCommand
from ....core.context import Context
from ...common import (
    add_concise_arg,
    add_source_scope_arg,
    add_tidy_build_dir_arg,
    add_tidy_config_args,
    add_tidy_task_view_arg,
)
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used to resolve the current tidy source scope.",
    )
    add_tidy_build_dir_arg(parser)
    add_tidy_config_args(parser)
    parser.add_argument("--jobs", type=int, default=None, help="Ninja jobs for full tidy.")
    parser.add_argument("--dry-run", action="store_true", help="Preview queue regeneration.")
    add_tidy_task_view_arg(parser)
    add_concise_arg(parser)
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--keep-going", dest="keep_going", action="store_true", default=None)
    group.add_argument("--no-keep-going", dest="keep_going", action="store_false")


def run(args: argparse.Namespace, ctx: Context) -> int:
    return TidyRefreshCommand(ctx).execute(
        app_name=args.app,
        source_scope=args.source_scope,
        build_dir_name=args.tidy_build_dir,
        task_view=args.task_view,
        dry_run=bool(args.dry_run),
        jobs=args.jobs,
        keep_going=args.keep_going,
        concise=bool(args.concise),
        config_file=args.config_file,
        strict_config=bool(args.strict_config),
    )


COMMAND = CommandSpec(name="tidy-refresh", register=register, run=run)
