import argparse

from ...commands.tidy.clean import CleanCommand
from ...core.context import Context
from ..model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    parser.add_argument(
        "--strict",
        action="store_true",
        help="Require latest verify result to be successful before archiving tasks.",
    )
    parser.add_argument(
        "--batch-id",
        default=None,
        help="Restrict clean scope to one batch (e.g. 1, 001, batch_001).",
    )
    parser.add_argument(
        "--cluster-by-file",
        action="store_true",
        help=(
            "Expand specified task id(s) to all tasks in the same batch "
            "that point to the same source file."
        ),
    )
    parser.add_argument("task_ids", nargs="+")


def run(args: argparse.Namespace, ctx: Context) -> int:
    cmd = CleanCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        task_ids=args.task_ids,
        strict=args.strict,
        batch_id=args.batch_id,
        cluster_by_file=args.cluster_by_file,
    )


COMMAND = CommandSpec(name="clean", register=register, run=run)
