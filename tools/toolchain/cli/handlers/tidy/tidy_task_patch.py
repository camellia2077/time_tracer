import argparse

from ....commands.tidy.task_patch import TidyTaskPatchCommand
from ....core.context import Context
from ...common import add_required_task_log_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    _ = defaults
    add_required_task_log_arg(parser)
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Return non-zero when preview generation hits failed candidates.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyTaskPatchCommand(ctx)
    return cmd.execute(
        task_log_path=args.task_log,
        strict=args.strict,
    )


COMMAND = CommandSpec(
    name="tidy-task-patch",
    register=register,
    run=run,
    app_mode="none",
)
