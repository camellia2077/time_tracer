import argparse

from ...commands.tidy.execution.clean import CleanCommand
from ...core.context import Context
from ..common import add_tidy_build_dir_arg
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    add_tidy_build_dir_arg(parser)
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Require latest verify result to be successful before archiving tasks.",
    )
    parser.add_argument(
        "--cluster-id",
        required=True,
        help="Source cluster directory under tasks/.",
    )
    parser.add_argument("task_ids", nargs="+")


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = CleanCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        task_ids=args.task_ids,
        strict=args.strict,
        cluster_id=args.cluster_id,
        tidy_build_dir_name=args.tidy_build_dir,
    )


COMMAND = CommandSpec(name="clean", register=register, run=run)
