import argparse

from ....commands.tidy.task_suggest import TidyTaskSuggestCommand
from ....core.context import Context
from ...common import add_source_scope_arg, add_task_selector_args, add_tidy_build_dir_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used to resolve the scoped tidy workspace for suggestion reports.",
    )
    add_tidy_build_dir_arg(parser)
    add_task_selector_args(parser)


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyTaskSuggestCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        task_log_path=args.task_log,
        batch_id=args.batch_id,
        task_id=args.task_id,
        tidy_build_dir_name=args.tidy_build_dir,
        source_scope=args.source_scope,
    )


COMMAND = CommandSpec(name="tidy-task-suggest", register=register, run=run)
