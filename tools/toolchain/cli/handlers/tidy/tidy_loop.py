import argparse

from ....commands.tidy.loop import TidyLoopCommand
from ....core.context import Context
from ...common import add_tidy_build_dir_arg
from ...model import CommandSpec, ParserDefaults


def register(parser: argparse.ArgumentParser, _: ParserDefaults) -> None:
    add_tidy_build_dir_arg(parser)
    tidy_loop_target_group = parser.add_mutually_exclusive_group()
    tidy_loop_target_group.add_argument(
        "--n",
        type=int,
        default=None,
        help="Max tasks to clean in this run (default: 1)",
    )
    tidy_loop_target_group.add_argument(
        "--all",
        action="store_true",
        help="Process tasks until none remain (or manual task blocks)",
    )
    parser.add_argument(
        "--test-every",
        type=int,
        default=1,
        help="Run verify after every K cleaned tasks",
    )
    parser.add_argument(
        "--concise",
        action="store_true",
        help="Use concise test output when supported",
    )
    parser.add_argument(
        "--kill-build-procs",
        action="store_true",
        help="Kill cmake/ninja/ccache before loop verify builds (default: off)",
    )
    parser.add_argument(
        "--no-kill-build-procs",
        action="store_true",
        help=argparse.SUPPRESS,
    )


def run(args: argparse.Namespace, ctx: Context) -> int:
    kill_build_procs = bool(args.kill_build_procs and not args.no_kill_build_procs)
    effective_n = args.n if args.n is not None else 1
    cmd = TidyLoopCommand(ctx)
    return cmd.execute(
        app_name=args.app,
        n=effective_n,
        process_all=args.all,
        test_every=args.test_every,
        concise=args.concise,
        tidy_build_dir_name=args.tidy_build_dir,
        kill_build_procs=kill_build_procs,
    )


COMMAND = CommandSpec(name="tidy-loop", register=register, run=run)
