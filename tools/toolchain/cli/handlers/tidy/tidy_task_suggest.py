import argparse

from ....commands.tidy.tasking.task_suggest import TidyTaskSuggestCommand
from ....core.context import Context
from ...common import add_required_task_log_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    _ = defaults
    add_required_task_log_arg(parser)


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyTaskSuggestCommand(ctx)
    return cmd.execute(task_log_path=args.task_log)


COMMAND = CommandSpec(
    name="tidy-task-suggest",
    register=register,
    run=run,
    app_mode="none",
)
