import argparse

from ....commands.tidy.task_fix import TidyTaskFixCommand
from ....core.context import Context
from ...common import add_required_task_log_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    _ = defaults
    add_required_task_log_arg(parser)
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
        task_log_path=args.task_log,
        dry_run=args.dry_run,
        strict=args.strict,
    )


COMMAND = CommandSpec(
    name="tidy-task-fix",
    register=register,
    run=run,
    app_mode="none",
)
