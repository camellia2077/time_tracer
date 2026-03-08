import argparse

from ....commands.tidy.task_patch import TidyTaskPatchCommand
from ....core.context import Context
from ...common import add_source_scope_arg, add_task_selector_args, add_tidy_build_dir_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, defaults: ParserDefaults) -> None:
    add_source_scope_arg(
        parser,
        defaults,
        help_suffix="Used to resolve scoped tidy source roots for patch preview generation.",
    )
    add_tidy_build_dir_arg(parser)
    add_task_selector_args(parser)
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Return non-zero when preview generation hits failed candidates.",
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = TidyTaskPatchCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        task_log_path=args.task_log,
        batch_id=args.batch_id,
        task_id=args.task_id,
        tidy_build_dir_name=args.tidy_build_dir,
        source_scope=args.source_scope,
        strict=args.strict,
    )


COMMAND = CommandSpec(name="tidy-task-patch", register=register, run=run)
