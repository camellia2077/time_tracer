import argparse

from ....commands.tidy.task_fix import TidyTaskFixCommand
from ....core.context import Context
from ...common import add_source_scope_arg, add_task_selector_args, add_tidy_build_dir_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used to resolve scoped tidy source roots for task-local fixes.",
    )
    add_tidy_build_dir_arg(parser)
    add_task_selector_args(parser)
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Preview safe auto-fixes and generate reports without modifying source files.",
    )
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Return non-zero when any supported action fails.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyTaskFixCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        task_log_path=args.task_log,
        batch_id=args.batch_id,
        task_id=args.task_id,
        tidy_build_dir_name=args.tidy_build_dir,
        source_scope=args.source_scope,
        dry_run=args.dry_run,
        strict=args.strict,
    )


COMMAND = CommandSpec(name="tidy-task-fix", register=register, run=run)
